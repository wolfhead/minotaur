/**
 * @file client_channel.cpp
 * @author Wolfhead
 */
#include "client_channel.h"
#include "socket_op.h"
#include "channel.h"
#include "../io_service.h"
#include "../stage.h"
#include "../common/scope_guard.h"
#include "../service/service_handler.h"
#include "io_handler.h"
#include "io_descriptor_factory.h"
#include "async_sequence_keeper.h"
#include "protocol/protocol.h"

namespace minotaur {

LOGGER_CLASS_IMPL_NAME(logger, ClientChannel, "net.ClientChannel");

ClientChannel::ClientChannel(
    IOService* io_service, 
    uint32_t timeout_msec)
    : Channel(io_service, -1) 
    , status_(kBroken)
    , reconnect_timer_(0)
    , sequence_keeper_(timeout_msec) {
}

ClientChannel::~ClientChannel() {
}

int ClientChannel::Start() {
  return TryConnect();
}

int ClientChannel::Stop() {
  SendEventMessage(
      EventMessage(
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
  CancelTimer();

  if (GetStatus() == kConnected) {
    MI_LOG_WARN(logger, "ClientChannel::OnClose Connection lost");
    reconnect_timer_ = IOHandler::GetCurrentIOHandler()->StartTimer(
        5000, 
        std::bind(&ClientChannel::TryConnect, this));
  } else if (GetStatus() == kConnecting || GetStatus() == kBroken) {
    MI_LOG_WARN(logger, "ClientChannel::OnClose Connector retry in 5 seconds");
    reconnect_timer_ = IOHandler::GetCurrentIOHandler()->StartTimer(
        5000, 
        std::bind(&ClientChannel::TryConnect, this));
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
        << ", error:" << SystemError::FormatMessage(connect_status));
    return;
  }
  
  if (0 != RegisterReadWrite()) {
    MI_LOG_ERROR(logger, "ClientChannel::OnConnect RegisterReadWrite fail");
    return;
  }

  guard.Dispose();
  SetStatus(kConnected);

  MI_LOG_TRACE(logger, "ClientChannel::OnWrite client connected on channel:"
      << GetDiagnositicInfo());

  Channel::OnWrite();
}

int ClientChannel::EncodeMessage(ProtocolMessage* message) {
  if (GetStatus() != kConnected) {
    MI_LOG_DEBUG(logger, "ClientChannel::EncodeMessage ChannelBroker:" << GetStatus());
    return -1;
  }

  message->sequence_id = sequence_keeper_.GenerateSequenceId();

  if (!GetProtocol()->Encode(this, &write_buffer_, message)) {
    MI_LOG_ERROR(logger, "ClientChannel::EncodeMessage fail");
    return -1;
  }

  if (0 != sequence_keeper_.Register(message)) {
    MI_LOG_WARN(logger, "ClientChannel::OnEncodeMessage Register fail");
    return -1;
  }

  return 0;
}

void ClientChannel::OnDecodeMessage(ProtocolMessage* message) {
  ProtocolMessage* keeper_message = sequence_keeper_.Fetch(message->sequence_id);
  if (!keeper_message) {
    MI_LOG_DEBUG(logger, "ClientChannel::OnDecodeMessage sequence not found" 
        << ", sequence_id:" << message->sequence_id
        << ", client_channel:" << GetDiagnositicInfo());
    MessageFactory::Destroy(message);
    return;
  }

  message->status = ProtocolMessage::kStatusOK;
  message->direction = ProtocolMessage::kIncomingResponse;
  message->handler_id = keeper_message->handler_id;
  message->sequence_id = keeper_message->sequence_id;
  message->descriptor_id = GetDescriptorId();
  message->payload = keeper_message->payload;

  MI_LOG_TRACE(logger, "ClientChannel::OnDecodeMessage " << *message);

  MessageFactory::Destroy(keeper_message);

  if (!GetIOService()->GetServiceStage()->Send(message)) {
    MI_LOG_WARN(logger, "Channel::DecodeMessage Send message fail");
    MessageFactory::Destroy(message);
    return;
  }

}

int ClientChannel::TryConnect() {
  reconnect_timer_ = 0;

  if (GetStatus() != kBroken) {
    MI_LOG_WARN(logger, "ClientChannel::TryConnect connector stop status:" << GetStatus());
    return -1;
  }

  SetStatus(kConnecting);

  if (GetFD() != -1) {
    close(GetFD());
    SetFD(-1);
  }

  if (0 != SocketOperation::GetSocketAddress(GetIp(), GetPort(), &sock_addr_)) {
    MI_LOG_ERROR(logger, "ClientChannel::TryConnect GetSocketAddress failed"
        << ", ip:" << GetIp()
        << ", port:" << GetPort());
    return -1;
  }

  int fd = SocketOperation::CreateTcpSocket();
  if (fd < 0) {
    MI_LOG_ERROR(logger, "ClientChannel::TryConnect CreateTcpSocket, failed with:" 
        << SystemError::FormatMessage());
    return -1;
  }
  SetFD(fd);

  ScopeGuard guard([=] {
      close(fd); 
      SetFD(-1);
      SetStatus(kBroken);});

  if (0 != SocketOperation::SetNonBlocking(fd)) {
    MI_LOG_ERROR(logger, "ClientChannel::TryConnect SetNonBlocking, failed with:" 
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

void ClientChannel::CancelTimer() {
  if (reconnect_timer_ != 0) {
    IOHandler::GetCurrentIOHandler()->CancelTimer(reconnect_timer_);
    reconnect_timer_ = 0;
  }
}

} //namespace minotaur
