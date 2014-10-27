/**
 * @file channel.cpp
 * @author Wolfhead
 */
#include "channel.h"
#include "socket_op.h"

namespace minotaur {

LOGGER_CLASS_IMPL_NAME(logger, Channel, "net.Channel");

Channel::Channel(IOService* io_service, int fd) 
    : Socket(io_service, fd) {
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
     << "}";
}

int Channel::Start() {
  if (0 != SocketOperation::SetNonBlocking(GetFD())) {
    LOG_WARN(logger, "Channel::Start SetNonBlocking fail"
        << ", channel:" << GetDiagnositicInfo());
    return -1;
  }

  if (0 != RegisterRead()) {
    LOG_WARN(logger, "Channel::Start RegisterRead fail"
        << ", channel:" << GetDiagnositicInfo());
    return -1;
  }

  if (0 != RegisterWrite()) {
    LOG_WARN(logger, "Channel::Start RegisterWrite fail"
        << ", channel:" << GetDiagnositicInfo());
    return -1;
  }

  return 0;
}

void Channel::ReadBuffer() {
  int ret = 0;
  while (true) {
    ret = SocketOperation::Receive(GetFD(), read_buffer_.EnsureWrite(1024), 1024);
    if (ret <= 0) {
      break;
    }
    read_buffer_.Produce(ret);

    //TODO
    // should check buffer size
  }

  if (ret == 0) {
    LOG_DEBUG(logger, "Channel::ReadBuffer channel closed by peer" 
        << ", channel:" << GetDiagnositicInfo());
    SocketOperation::ShutDownRead(GetFD());
  } else if (!SocketOperation::WouldBlock(SystemError::Get())) {
    LOG_DEBUG(logger, "Channel::ReadBuffer channel read fail"
        << ", error:" << SystemError::FormatMessage()
        << ", channel:" << GetDiagnositicInfo()
        << ", ret:" << ret);
    SocketOperation::ShutDownBoth(GetFD());
  }

  //TODO
  //Codec
  //Test as echo
  
  std::string line(read_buffer_.GetRead(), read_buffer_.GetReadSize());
  LOG_TRACE(logger, "line:[" << line << "]");
  if (line.find("quit") == 0) {
    LOG_TRACE(logger, "Close");
    SocketOperation::ShutDownWrite(GetFD());
    return;
  }

  memcpy(
      write_buffer_.EnsureWrite(read_buffer_.GetReadSize()),
      read_buffer_.GetRead(),
      read_buffer_.GetReadSize());
  write_buffer_.Produce(read_buffer_.GetReadSize());
  read_buffer_.Consume(read_buffer_.GetReadSize());
  WriteBuffer();
}

void Channel::WriteBuffer() {
  uint32_t data_size = 0;
  int ret = 0;

  while ((data_size = write_buffer_.GetReadSize()) != 0) {
    ret = SocketOperation::Send(GetFD(), write_buffer_.GetRead(), data_size);
    if (ret <= 0) {
      break;
    }
    write_buffer_.Consume(ret);
  }

  if (ret < 0 && !SocketOperation::WouldBlock(SystemError::Get())) {
    LOG_DEBUG(logger, "Channel::WriteBuffer fail"
        << ", error:" << SystemError::FormatMessage()
        << ", channel:" << GetDiagnositicInfo());
    SocketOperation::ShutDownWrite(GetFD());
  }
}

void Channel::OnRead(event::EventLoop* event_loop) {
  //TODO
  // currently for testing
  ReadBuffer();
}

void Channel::OnWrite(event::EventLoop* event_loop) {
  //TODO
  //currently for testing
  WriteBuffer();
}

void Channel::OnClose(event::EventLoop* event_loop) {
  Socket::OnClose(event_loop);
  delete this; 
}

} //namespace minotaur
