/**
 * @file socket.cpp
 * @author Wolfhead
 */
#include "socket.h"
#include <sys/socket.h>
#include "../event/event_loop_data.h"
#include "../common/system_error.h"
#include "../io_service.h"

namespace minotaur {

LOGGER_CLASS_IMPL_NAME(logger, Socket, "net.Socket");

Socket::Socket(IOService* io_service) 
    : io_service_(io_service)
    , fd_(-1) {
}

Socket::Socket(IOService* io_service, int fd) 
    : io_service_(io_service)
    , fd_(fd) {
}

Socket::~Socket() {
}

void Socket::SocketCommonProc(
    event::EventLoop* event_loop,
    int fd,
    void* data,
    uint32_t mask) {
  MI_LOG_TRACE(logger, "SocketCommonProc called"
      << ", fd:" << fd
      << ", data:" << data
      << ", mask:" << mask);

  if (!data) {
    MI_LOG_FATAL(logger, "Socket::SocketCommonProc data is NULL");
    return;
  }

  Socket* sock = static_cast<Socket*>(data);
  if (sock->GetFD() != fd) {
    MI_LOG_FATAL(logger, "Socket::SocketCommonProc fd mismatch"
        << ", incoming:" << fd
        << ", current:" << sock->GetFD());
    return;
  }

  if (mask & event::EventType::EV_READ) {
    sock->OnRead(event_loop);
  } 

  if (mask & event::EventType::EV_WRITE) {
    sock->OnWrite(event_loop);    
  }

  if (mask & event::EventType::EV_CLOSE) {
    MI_LOG_TRACE(logger, "SocketCommonProc EV_CLOSE, " << *sock);
    event_loop->RemoveEvent(fd, 0xFFFFFFFF);
    sock->OnClose(event_loop); 
    return;
  }
}

int Socket::RegisterRead() {
  return GetIOService()->GetEventLoopStage()->RegisterRead(
      GetFD(), 
      &Socket::SocketCommonProc,
      this);
}

int Socket::RegisterWrite() {
  return GetIOService()->GetEventLoopStage()->RegisterWrite(
      GetFD(),
      &Socket::SocketCommonProc,
      this);
}

void Socket::OnRead(event::EventLoop* event_loop) {
}

void Socket::OnWrite(event::EventLoop* event_loop) {
}

void Socket::OnClose(event::EventLoop* event_loop) {
  close(fd_);
  fd_ = -1;
}

void Socket::OnProcFinish(event::EventLoop* event_loop) {
}

void Socket::Close() {
  if (fd_) {
    shutdown(fd_, SHUT_RDWR);
  }
}

void Socket::Dump(std::ostream& os) const {
  os << "[Socket fd:" << fd_ << "]";
}

std::string Socket::ToString() const {
  std::ostringstream oss;
  Dump(oss);
  return oss.str();
}

std::ostream& operator << (std::ostream& os, const Socket& sock) {
  sock.Dump(os);
  return os;
}

} //namespace minotaur
