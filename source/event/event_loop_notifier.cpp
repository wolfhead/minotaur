/**
 * @file event_loop_notifier.cpp
 * @author Wolfhead
 */
#include "event_loop_notifier.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/eventfd.h>
#include <pthread.h>
#include "event_loop.h"
#include "../common/system_error.h"
#include "../common/thread_id.h"
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
    , event_fd_(-1)
    , queue_(1024 * 1024) 
    , working_thread_id_(0) {
}

EventLoopNotifier::~EventLoopNotifier() {
  if (event_fd_ != -1) {
    close(event_fd_);
    event_fd_ = -1;
  }
}

int EventLoopNotifier::Init() {
  if (event_fd_ != -1) {
    return 0;
  }

  int event_fd = -1;
  if (-1 == (event_fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK))) {
    LOG_ERROR(logger, "EventLoopThread::CreateEventFd fail with:"
        << SystemError::FormatMessage());
    return -1;
  }

  if (0 != event_loop_->AddEvent(
        event_fd, 
        EventType::EV_READ, 
        &EventLoopNotifier::EventLoopNotifierProc,
        &queue_)) {
    LOG_ERROR(logger, "EventLoopNotifier::Init AddEvent fail");
    close(event_fd);
    return -1;
  }

  event_fd_ = event_fd;
  return 0;
}

int EventLoopNotifier::Notify(int fd, uint32_t mask, FdEventProc* proc, void* data) {
  static const uint64_t add_count = 1;

  NotifyMessage message = {
    .fd = fd,
    .mask = mask,
    .data = data,
    .proc = proc,
  };

  if (ThreadId::Get() == working_thread_id_) {
    return NotifyEventLoop(event_loop_, message);
  }

  if (!queue_.Push(message)) {
    LOG_FATAL(logger, "EventLoopNotifier::Notify queue fail"
        << ", fd:" << fd);
    return -1;
  }

  if (SocketOperation::Send(event_fd_, &add_count, sizeof(uint64_t)) != sizeof(uint64_t)) {
    LOG_FATAL(logger, "EventLoopNotifier::Notify fail"
        << ", event_fd:" << event_fd_
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

int EventLoopNotifier::RegisterClose(int fd, void* data) {
  return Notify(fd, NotifyMessage::REGISTER_CLOSE, NULL, data);
}

int EventLoopNotifier::UnregisterRead(int fd) {
  return Notify(fd, NotifyMessage::REMOVE_READ, NULL, NULL); 
}

int EventLoopNotifier::UnregisterWrite(int fd) {
  return Notify(fd, NotifyMessage::REMOVE_WRITE, NULL, NULL); 
}

int EventLoopNotifier::Process() {
  EventLoopNotifierProc(event_loop_, event_fd_, &queue_, 0);
  return 0;
}

void EventLoopNotifier::EventLoopNotifierProc(
    EventLoop* event_loop,
    int fd,
    void* data,
    uint32_t mask) {
  uint64_t queue_count = 0;
  SocketOperation::Receive(fd, &queue_count, sizeof(uint64_t));
  MessageQueueType* queue = (MessageQueueType*)data;

    
  NotifyMessage message;
  while (queue->Pop(&message)) {
    NotifyEventLoop(event_loop, message);
  }
}

int EventLoopNotifier::NotifyEventLoop(
    EventLoop* event_loop, 
    const NotifyMessage& message) {

  IODescriptor* descriptor = NULL;
  if (message.fd == kDescriptorFD) {
    uint64_t descriptor_id = (uint64_t)message.data;
    descriptor = IODescriptorFactory::GetIODescriptor(descriptor_id);
    if (!descriptor) {
      LOG_ERROR(logger, "EventLoopNotifier::NotifyEventLoop descriptor not found:" 
          << descriptor_id);
      return -1;
    }
  }
  
  LOG_TRACE(logger, "EventLoopNotifier::NotifyEventLoop"
      << ", message:" << message
      << ", descriptor:" << *descriptor);

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
    case NotifyMessage::REGISTER_CLOSE:
      event_loop->FireEvent(descriptor->GetIN(), EventType::EV_CLOSE);
      return 0;
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
