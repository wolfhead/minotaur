/**
  @file event_loop.cpp
  @author Wolfhead
*/
#include "event_loop.h"
#include <algorithm>
#include "impl/event_loop_impl.h"
#include "../common/system_error.h"

namespace minotaur { namespace event {

LOGGER_CLASS_IMPL_NAME(logger, EventLoop, "event.EventLoop");

EventLoop::EventLoop() {
}

EventLoop::~EventLoop() {
  Destroy();
}

int EventLoop::Init(uint32_t fd_size) {
  data_.Init(fd_size);

  if (0 != EventLoopImpl::Init(&data_)) {
    return -1;
  }

  return 0;
}

int EventLoop::Destroy() {
  EventLoopImpl::Destroy(&data_);
  return 0;
}

int EventLoop::Stop() {
  return 0;
}

int EventLoop::AddEvent(
    int fd, 
    uint32_t mask, 
    FdEventProc* proc, 
    void* client_data) {

  if ((uint32_t)fd >= data_.fd_size) {
    LOG_ERROR(logger, "EventLoop::AddEvent fd too large"
        << ", fd:" << fd
        << ", fd_size:" << data_.fd_size);
    return -1;
  }

  FdEvent* fe = &data_.fd_events[fd];

  if (EventLoopImpl::AddEvent(&data_, fd, mask) != 0) {
    return -1;
  }

  fe->mask |= mask;
  fe->proc = proc;
  fe->client_data = client_data;

  return 0;
}

int EventLoop::RemoveEvent(int fd, uint32_t mask) {
  if ((uint32_t)fd >= data_.fd_size) {
    LOG_ERROR(logger, "EventLoop::RemoveEvent fd too large"
        << ", fd:" << fd
        << ", fd_size:" << data_.fd_size);
    return -1;
  }

  FdEvent* fe = &data_.fd_events[fd];

  if (fe->mask == EventType::EV_NONE) {
    return 0;
  }

  return EventLoopImpl::RemoveEvent(&data_, fd, mask);
}

int EventLoop::DeleteEvent(int fd) {
  if ((uint32_t)fd >= data_.fd_size) {
    LOG_ERROR(logger, "EventLoop::DeleteEvent fd too large"
        << ", fd:" << fd
        << ", fd_size:" << data_.fd_size);
    return -1;
  }

  FdEvent* fe = &data_.fd_events[fd];
  memset(fe, 0, sizeof(FdEvent));

  return EventLoopImpl::RemoveEvent(&data_, fd, 0xFFFFFFFF);
}

int EventLoop::ProcessEvent(uint32_t timeout) {
  int ret = EventLoopImpl::Poll(&data_, timeout);
  if (ret < 0) {
    LOG_ERROR(logger, "EventLoop::ProcessEvent failed with error:"
        << SystemError::FormatMessage());
    return ret;
  }

  for (int i = 0; i != ret; i++) {
    FiredEvent* fired_event = &data_.fired_events[i];
    int mask = fired_event->mask;
    int fd = fired_event->fd;
    FdEvent* file_event = &data_.fd_events[fd];

    if (file_event->mask & mask) {
      file_event->proc(this, fd, file_event->client_data, mask);
    }
  }

  return ret;
}

const char* EventLoop::GetImplement() const {
  return EventLoopImpl::GetImplement();
}

} //namespace event
} //namespace minotaur
