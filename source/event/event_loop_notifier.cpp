/**
 * @file event_loop_notifier.cpp
 * @author Wolfhead
 */
#include "event_loop_notifier.h"
#include <sys/types.h>
#include <sys/socket.h>
#include "event_loop.h"
#include "../common/system_error.h"
#include "../net/socket_op.h"
#include "../net/io_descriptor.h"
#include "../net/io_descriptor_factory.h"

namespace minotaur { namespace event {

LOGGER_CLASS_IMPL_NAME(logger, EventLoopNotifier, "event.EventLoopNotifier");

void NotifyMessage::Dump(std::ostream& os) const {
  os << "{\"type\": \"NotifyMessage\""
     << ", \"fd\": " << fd
     << ", \"mask\": " << mask
     << ", \"data\": " << data
     << ", \"proc\": " << proc << "}";
}

EventLoopNotifier::EventLoopNotifier(
    EventLoop* event_loop) 
    : event_loop_(event_loop)
    , notifier_in_(-1)
    , notifier_out_(-1) {
}

EventLoopNotifier::~EventLoopNotifier() {
  if (notifier_in_ != -1) {
    close(notifier_in_);
    notifier_in_ = -1;
  }

  if (notifier_out_ != -1) {
    close(notifier_out_);
    notifier_out_ = -1;
  }
}

int EventLoopNotifier::Init() {
  if (notifier_in_ != -1 || notifier_out_ != -1) {
    return 0;
  }

  int fd[2];
  if (0 != socketpair(AF_UNIX, SOCK_STREAM, 0, fd)) {
    LOG_ERROR(logger, "EventLoopThread::CreateSocketPair fail with:"
        << SystemError::FormatMessage());
    return -1;
  }

  int notifier_in = fd[0];
  int notifier_out = fd[1];

  if (0 != SocketOperation::SetNonBlocking(notifier_out)) {
    LOG_ERROR(logger, "EventLoopNotifier::Init SetNonBlocking fail with:"
        << SystemError::FormatMessage());
    close(notifier_in);
    close(notifier_out);
    return -1;
  }
 
  if (0 != event_loop_->AddEvent(
        notifier_out, 
        EventType::EV_READ, 
        &EventLoopNotifier::EventLoopNotifierProc,
        NULL)) {
    LOG_ERROR(logger, "EventLoopNotifier::Init AddEvent fail");
    close(notifier_in);
    close(notifier_out);
    return -1;
  }

  notifier_in_ = notifier_in;
  notifier_out_ = notifier_out;

  return 0;
}

int EventLoopNotifier::Notify(int fd, uint32_t mask, FdEventProc* proc, void* data) {
  NotifyMessage message = {
    .fd = fd,
    .mask = mask,
    .data = data,
    .proc = proc,
  };

  // TODO this is a blocking operation
  // can we make it nonblocking
  if (SocketOperation::Send(notifier_in_, &message, sizeof(message)) != sizeof(message)) {
    LOG_FATAL(logger, "EventLoopNotifier::Notify fail"
        << ", notify_in:" << notifier_in_
        << ", fd:" << fd
        << ", error:" << SystemError::FormatMessage());
    return -1;
  }

  return 0;
}

int EventLoopNotifier::RegisterRead(
    int fd, 
    FdEventProc* proc, 
    void* data) {
  return Notify(fd, NotifyMessage::ADD_READ, proc, data); 
}

int EventLoopNotifier::RegisterWrite(
    int fd, 
    FdEventProc* proc, 
    void* data) {
  return Notify(fd, NotifyMessage::ADD_WRITE, proc, data); 
}

int EventLoopNotifier::RegisterReadWrite(
    int fd, 
    FdEventProc* proc, 
    void* data) {
  return Notify(fd, NotifyMessage::ADD_READ_WRITE, proc, data); 
}

int EventLoopNotifier::RegisterClose(int fd) {
  return Notify(fd, NotifyMessage::ADD_CLOSE, NULL, NULL);
}

int EventLoopNotifier::UnregisterRead(int fd) {
  return Notify(fd, NotifyMessage::REMOVE_READ, NULL, NULL); 
}

int EventLoopNotifier::UnregisterWrite(int fd) {
  return Notify(fd, NotifyMessage::REMOVE_WRITE, NULL, NULL); 
}

void EventLoopNotifier::EventLoopNotifierProc(
    EventLoop* event_loop,
    int fd,
    void* data,
    uint32_t mask) {
  LOG_TRACE(logger, "EventLoopNotifier::EventLoopNotifierProc start");

  NotifyMessage message;
  uint32_t ret = 0;
  for (;;) {
    ret = SocketOperation::Receive(fd, &message, sizeof(message));
    if (ret != sizeof(message)) {
      if (ret == 0) {
        LOG_INFO(logger, "EventLoopNotifier::EventLoopNotifierProc notifier"
                          " socket closed:");        
        return;
      } else if (SocketOperation::WouldBlock(SystemError::Get())) {
        return;
      } else {
        LOG_FATAL(logger, "EventLoopNotifier::EventLoopNotifierProc notifier"
                          " socket read with error:" 
                          << SystemError::FormatMessage());
        return;
      }
    }

    NotifyEventLoop(event_loop, message);
  }
}

int EventLoopNotifier::NotifyEventLoop(
    EventLoop* event_loop, 
    const NotifyMessage& message) {
  LOG_TRACE(logger, "EventLoopNotifier::NotifyEventLoop message:"
      << message);

  IODescriptor* descriptor = NULL;
  if (message.fd == kDescriptorFD) {
    uint64_t descriptor_id = (uint64_t)message.data;
    descriptor = IODescriptorFactory::Instance().
        GetIODescriptor(descriptor_id);
    if (!descriptor) {
      LOG_ERROR(logger, "EventLoopNotifier::NotifyEventLoop descriptor not found:" 
          << descriptor_id);
      return -1;
    }
  }


  //TODO
  //fix these in out
  switch (message.mask) {
    case NotifyMessage::ADD_READ:
      return event_loop->AddEvent(
          descriptor ? descriptor->GetIN() : message.fd, 
          EventType::EV_READ, 
          message.proc, 
          message.data);
    case NotifyMessage::ADD_WRITE:
      return event_loop->AddEvent(
          descriptor ? descriptor->GetOUT() : message.fd, 
          EventType::EV_WRITE, 
          message.proc, 
          message.data);
    case NotifyMessage::ADD_READ_WRITE:
      return event_loop->AddEvent(
          descriptor ? descriptor->GetIN() : message.fd, 
          EventType::EV_READ | EventType::EV_WRITE, 
          message.proc, 
          message.data);
    case NotifyMessage::REMOVE_READ:
      return event_loop->RemoveEvent(
          descriptor ? descriptor->GetIN() : message.fd, 
          EventType::EV_READ);
    case NotifyMessage::REMOVE_WRITE:
      return event_loop->RemoveEvent(
          descriptor ? descriptor->GetOUT() : message.fd, 
          EventType::EV_WRITE);
    default:
      LOG_WARN(logger, "EventLoopNotifier::NotifyEventLoop unknown mask:" << message.mask);
      break;
  }
  return 0;
}

std::ostream& operator << (std::ostream& os, const NotifyMessage& notify_message) {
  notify_message.Dump(os);
  return os;
}

} //namespace event
} //namespace minotaur
