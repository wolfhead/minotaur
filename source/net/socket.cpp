/**
 * @file socket.cpp
 * @author Wolfhead
 */
#include "socket.h"
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
  LOG_TRACE(logger, "SocketCommonProc called");

  if (!data) {
    LOG_FATAL(logger, "Socket::SocketCommonProc data is NULL");
    return;
  }

  Socket* sock = static_cast<Socket*>(data);
  if (sock->GetFD() != fd) {
    LOG_FATAL(logger, "Socket::SocketCommonProc fd mismatch"
        << ", incoming:" << fd
        << ", current:" << sock->GetFD());
    return;
  }

  if (mask | event::EventType::EV_READ) {
    sock->OnRead(event_loop);
  } 

  if (mask | event::EventType::EV_WRITE) {
    sock->OnWrite(event_loop);    
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

void Socket::OnProcFinish(event::EventLoop* event_loop) {
}

void Socket::Close(event::EventLoop* event_loop) {
  if (GetFD() == -1) {
    return;
  }

  if (0 != event_loop->DeleteEvent(GetFD())) {
    LOG_ERROR(logger, "Socket::Close DeleteEvent failed with:"
        << SystemError::FormatMessage());
  }

  close(GetFD());
  fd_ = -1;

  //TODO
  // how do we destroy this Socket?
}

} //namespace minotaur
