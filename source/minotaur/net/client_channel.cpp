/**
 * @file client_channel.cpp
 * @author Wolfhead
 */
#include "client_channel.h"
#include "socket_op.h"
#include "channel.h"
#include "protocol/protocol.h"
#include "../io_service.h"
#include "../stage.h"
#include "../common/scope_guard.h"
#include "../service/service_handler.h"
#include "io_handler.h"
#include "io_descriptor_factory.h"
#include "../matrix/matrix_scope.h"

namespace ade {

LOGGER_CLASS_IMPL_NAME(logger, ClientChannel, "net.ClientChannel");

ClientChannel::ClientChannel(
    IOService* io_service, 
    uint32_t timeout_msec,
    uint32_t heartbeat_msec)
    : Channel(io_service, -1) 
    , status_(kBroken)
    , timeout_ms_(timeout_msec)
    , heartbeat_ms_(heartbeat_msec)
    , reconnect_timer_(0)
    , timeout_timer_(0)
    , heartbeat_timer_(0) {
}

ClientChannel::~ClientChannel() {
}

int ClientChannel::Start() {
  return TryConnect();
}

int ClientChannel::StartReconnectTimer() {
  if (0 != reconnect_timer_) {
    MI_LOG_WARN(logger, "ClientChannel::StartReconnectTimer duplicate!");
    return -1;
  }
  reconnect_timer_ = IOHandler::GetCurrentIOHandler()->StartTimer(
      1000, 
      std::bind(&ClientChannel::TryConnect, this));
  return 0;
}

int ClientChannel::StartTimeoutTimer() {
  if (0 == timeout_ms_) {
    return 0;
  }

  timeout_timer_ = IOHandler::GetCurrentIOHandler()->StartTimer(
      5,
      std::bind(&ClientChannel::OnTimeout, this));
  return 0;
}

int ClientChannel::StartHeartBeatTimer() {
  if (0 == heartbeat_ms_) {
    return 0;
  }

  heartbeat_timer_ = IOHandler::GetCurrentIOHandler()->StartTimer(
      heartbeat_ms_,
      std::bind(&ClientChannel::OnHeartBeat, this));
  return 0;
}

int ClientChannel::Stop() {
  SendEventMessage(
      MessageFactory::Allocate<EventMessage>(
          MessageType::kIOActiveCloseEvent, 
          GetDescriptorId()));
  return 0;
}

void ClientChannel::OnWrite() {
  if (GetStatus() == kConnecting) {
    return OnConnect();
  } else {
    return Channel::OnWrite();
  }
}

void ClientChannel::OnClose() {
  PurgeSequenceKeeper();
  CancelTimer();
  ResetLocal();
  IODescriptor::Close();

  if (GetStatus() == kConnected) {
    MI_LOG_DEBUG(logger, "ClientChannel::OnClose Connection lost");
    SetStatus(kBroken);
    StartReconnectTimer();
  } else if (GetStatus() == kConnecting || GetStatus() == kBroken) {
    MI_LOG_DEBUG(logger, "ClientChannel::OnClose Connector retry in 1 seconds");
    SetStatus(kBroken);
    StartReconnectTimer();
  } else {
    MI_LOG_INFO(logger, "ClientChannel::OnClose Connection close");
    Channel::OnClose();
  }
}

void ClientChannel::OnActiveClose() {
  if (GetStatus() == kBroken) {
    SetStatus(kStopping);
    OnClose();
  } else {
    CancelTimer();
    SetStatus(kStopping);
    Channel::OnActiveClose();
  }
}

void ClientChannel::OnConnect() {
  MI_LOG_TRACE(logger, "ClientChannel::OnConnect ip:" << GetIp() << ", port:" << GetPort());

  ScopeGuard guard([=] {SetStatus(kBroken);});

  int connect_status;
  socklen_t connect_status_len = sizeof(connect_status);
  if (0 != getsockopt(GetFD(), SOL_SOCKET, SO_ERROR, &connect_status, &connect_status_len)) {
    MI_LOG_WARN(logger, "ClientChannel::OnWrite getsockopt fail" 
        << ", error:" << SystemError::FormatMessage());
    return;
  }

  if (connect_status != 0) {
    MI_LOG_WARN(logger, "ClientChannel::OnWrite fail" 
        << ", error:" << SystemError::FormatMessage(connect_status)
        << ", address:" << GetIp() << ":" << GetPort());
    return;
  }
  
  if (0 != RegisterReadWrite()) {
    MI_LOG_ERROR(logger, "ClientChannel::OnConnect RegisterReadWrite fail");
    return;
  }

  if (0 != StartTimeoutTimer()) {
    MI_LOG_ERROR(logger, "ClientChannel::OnConnect StartTimeoutTimer fail");
    return;
  }

  if (0 != StartHeartBeatTimer()) {
    MI_LOG_ERROR(logger, "ClientChannel::OnConnect StartHeartBeatTimer fail");
    return;
  }

  guard.Dispose();
  SetStatus(kConnected);

  MI_LOG_TRACE(logger, "ClientChannel::OnWrite client connected on channel:"
      << GetDiagnositicInfo());

  ResetLocal();
  Channel::OnWrite();
}

void ClientChannel::OnHeartBeat() {

  ProtocolMessage* heartbeat_message = GetProtocol()->HeartBeatRequest();
  if (0 != EncodeMessage(heartbeat_message)) {
    MessageFactory::Destroy(heartbeat_message);
  }

  OnWrite();

  StartHeartBeatTimer();
}

void ClientChannel::BreakChannel() {
  if (GetStatus() != kConnected) {
    return;
  }

  SetStatus(kBroken);
  SocketOperation::ShutDownBoth(GetFD());
  ResetLocal();
}

int ClientChannel::TryConnect() {
  bool is_reconnect = false;
  if (reconnect_timer_ != 0) {
    is_reconnect = true;
    reconnect_timer_ = 0;
  }

  // if this connection is not broken, we do not want to connect it again
  if (GetStatus() != kBroken) {
    MI_LOG_WARN(logger, "ClientChannel::TryConnect connector stop status:" << GetStatus());
    if (GetStatus() != kConnected) {
      StartReconnectTimer();
    }
    return -1;
  }

  SetStatus(kConnecting);

  // close the old fd, if we are reconnecting
  // the last OnClose should clean all the event already
  if (GetFD() != -1) {
    close(GetFD());
    SetFD(-1);
  }

  int fd = SocketOperation::CreateTcpSocket();
  SetFD(fd);

  ScopeGuard guard([&] {
      if (fd != - 1) {close(fd);} 
      if (is_reconnect) {StartReconnectTimer();}
      SetFD(-1);
      SetStatus(kBroken);});

  if (fd < 0) {
    MI_LOG_ERROR(logger, "ClientChannel::TryConnect CreateTcpSocket, failed with:" 
        << SystemError::FormatMessage());
    return -1;
  }
 
  if (0 != SocketOperation::GetSocketAddress(GetIp(), GetPort(), &sock_addr_)) {
    MI_LOG_ERROR(logger, "ClientChannel::TryConnect GetSocketAddress failed"
        << ", ip:" << GetIp()
        << ", port:" << GetPort());
    return -1;
  }

  if (0 != SocketOperation::SetNonBlocking(fd)) {
    MI_LOG_ERROR(logger, "ClientChannel::TryConnect SetNonBlocking, failed with:" 
        << SystemError::FormatMessage());
    return -1;
  }

  if (0 != SocketOperation::SetNoDelay(fd)) {
    MI_LOG_ERROR(logger, "ClientChannel::TryConnect SetNoDelay, failed with:"
        << SystemError::FormatMessage());
    return -1;
  }

  if (-1 != SocketOperation::Connect(fd, &sock_addr_)) {
    if (SystemError::Get() == EINPROGRESS) {
      MI_LOG_ERROR(logger, "ClientChannel::TryConnect Connect, in progress with:" 
          << SystemError::FormatMessage());
    } else {
      MI_LOG_ERROR(logger, "ClientChannel::TryConnect Connect, failed with:" 
          << SystemError::FormatMessage());
      return -1;
    }
  }
  
  if (0 != RegisterWrite()) {
    MI_LOG_ERROR(logger, "ClientChannel::TryConnect RegisterReadWrite fail");
    return -1;
  }

  guard.Dispose();
  return 0;
}

void ClientChannel::ResetLocal() {
  read_buffer_.Reset();
  write_buffer_.Reset(); 
  if (GetProtocol()) {
    GetProtocol()->Reset();
  }
}

void ClientChannel::CancelTimer() {
  if (reconnect_timer_ != 0) {
    IOHandler::GetCurrentIOHandler()->CancelTimer(reconnect_timer_);
    reconnect_timer_ = 0;
  }

  if (timeout_timer_ != 0) {
    IOHandler::GetCurrentIOHandler()->CancelTimer(timeout_timer_);
    timeout_timer_ = 0;
  }

  if (heartbeat_timer_ != 0) {
    IOHandler::GetCurrentIOHandler()->CancelTimer(heartbeat_timer_);
    heartbeat_timer_ = 0;
  }
}

void ClientChannel::DoSendBack(ProtocolMessage* message, int status) {
  if (message->type_id == MessageType::kHeartBeatMessage
      || message->direction == ProtocolMessage::kOneway) {
    MessageFactory::Destroy(message);
    return;
  }

  message->status = status;
  if (!GetIOService()->GetServiceStage()->Send(message)) {
    MI_LOG_WARN(logger, "ClientChannel::DoSendBack send fail:" << *message);
    MessageFactory::Destroy(message);
  }
}

} //namespace ade
