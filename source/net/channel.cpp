/**
 * @file channel.cpp
 * @author Wolfhead
 */
#include "channel.h"
#include "socket_op.h"
#include "../io_service.h"
#include "../message.h"
#include "../stage.h"
#include "io_handler.h"
#include "../service/service_handler.h"
#include "protocol/protocol.h"

namespace minotaur {

LOGGER_CLASS_IMPL_NAME(logger, Channel, "net.Channel");

Channel::Channel(IOService* io_service, int fd) 
    : Socket(io_service, fd, true) {
}

Channel::~Channel() {
}

std::string Channel::GetDiagnositicInfo() const {
  std::ostringstream oss;
  DumpDisgnosticInfo(oss);
  return oss.str();
}

void Channel::DumpDisgnosticInfo(std::ostream& os) const {
  os << "{\"type:\": \"net.Channel\""
     << ", \"ip\": \"" << ip_ << "\""
     << ", \"port\": " << port_ 
     << ", \"fd\": " << GetFD()
     << "}";
}

int Channel::Start() {
  if (0 != SocketOperation::SetNonBlocking(GetFD())) {
    LOG_WARN(logger, "Channel::Start SetNonBlocking fail"
        << ", channel:" << GetDiagnositicInfo());
    return -1;
  }

  if (0 != RegisterReadWrite()) {
    LOG_WARN(logger, "Channel::Start RegisterReadWrite fail"
        << ", channel:" << GetDiagnositicInfo());
    return -1;
  }

  return 0;
}

int Channel::Stop() {
  assert("no implement");
  return -1;
}

void Channel::OnRead() {
  const static uint32_t per_read_size = 4096;
  const static uint32_t decode_watermark = 4096;

  int ret = 0;
  while (true) {
    ret = SocketOperation::Receive(GetFD(), read_buffer_.EnsureWrite(per_read_size), per_read_size);
    if (ret <= 0) {
      if (ret == 0) {
        LOG_DEBUG(logger, "Channel::ReadBuffer channel closed by peer" 
            << ", channel:" << GetDiagnositicInfo());
        SocketOperation::ShutDownRead(GetFD());
      } else if (!SocketOperation::WouldBlock(SystemError::Get())) {
        LOG_WARN(logger, "Channel::ReadBuffer channel read fail"
            << ", error:" << SystemError::FormatMessage()
            << ", channel:" << GetDiagnositicInfo()
            << ", ret:" << ret);
        SocketOperation::ShutDownBoth(GetFD());
      } else if (SocketOperation::WouldBlock(SystemError::Get())) {
        //RegisterRead();
      }
      break;
    }

    read_buffer_.Produce(ret);
    if (read_buffer_.GetReadSize() >= decode_watermark) {
      if (0 != DecodeMessage()) {
        SocketOperation::ShutDownRead(GetFD());
        break;
      }
    }
  }

  if (0 != DecodeMessage()) {
    SocketOperation::ShutDownRead(GetFD());
  }
}

void Channel::OnWrite() {
  uint32_t data_size = 0;
  int ret = 0;

  LOG_TRACE(logger, "Channel::OnWrite size:" << write_buffer_.GetReadSize());


  while ((data_size = write_buffer_.GetReadSize()) != 0) {
    ret = SocketOperation::Send(GetFD(), write_buffer_.GetRead(), data_size);
    if (ret <= 0) {
      if (!SocketOperation::WouldBlock(SystemError::Get())) {
        LOG_WARN(logger, "Channel::WriteBuffer fail"
            << ", error:" << SystemError::FormatMessage()
            << ", channel:" << GetDiagnositicInfo());
        SocketOperation::ShutDownWrite(GetFD());
      } else if (SocketOperation::WouldBlock(SystemError::Get())) {
        //RegisterWrite();
      }

      break;
    }
    write_buffer_.Consume(ret);
  }
}

void Channel::OnClose() {
  Destroy();
}

int Channel::DecodeMessage() {
  int result = Protocol::kDecodeSuccess;
  while (result == Protocol::kDecodeSuccess) {
    ProtocolMessage* message = GetProtocol()->Decode(&read_buffer_, &result);
    if (!message) {
      continue;
    }

    OnDecodeMessage(message);
  }

  if (result == Protocol::kDecodeFail) {
    return -1;
  }

  return 0;
}

int Channel::EncodeMessage(ProtocolMessage* message) {
  if (Protocol::kEncodeSuccess != GetProtocol()->Encode(&write_buffer_, message)) {
    MI_LOG_ERROR(logger, "Channel::EncodeMessage fail");
    return -1;
  }

  MessageFactory::Destroy(message);
  return 0;
}

} //namespace minotaur
