/**
  @file event_loop.cpp
  @author Wolfhead
*/
#include "event_loop.h"
#include <algorithm>
#include "impl/event_loop_impl.h"

namespace minotaur {
namespace event {

EventLoop::EventLoop()
    : data_(NULL) {
}

EventLoop::~EventLoop() {
  Destroy();
}

int EventLoop::Init(int set_size) {
  data_ = new EventLoopData(set_size);

  if (0 != EventLoopImpl::Init(data_)) {
    return -1;
  }

  return 0;
}

int EventLoop::Destroy() {
  if (data_) {
    EventLoopImpl::Destroy(data_);
    delete data_;
    data_ = NULL;
  }

  return 0;
}

int EventLoop::Stop() {
  data_->stop = true;
  return 0;
}

int EventLoop::AddEvent(
    int fd, 
    int mask, 
    FileEventProc* proc, 
    void* client_data) {

  if (fd >= data_->set_size) {
    return -1;
  }

  FileEvent* fe = &data_->file_events[fd];

  if (EventLoopImpl::AddEvent(data_, fd, mask) != 0) {
    return -1;
  }

  fe->mask |= mask;
  fe->proc = proc;
  fe->client_data = client_data;

  data_->max_fd = std::max(data_->max_fd, fd);

  return 0;
}

int EventLoop::RemoveEvent(int fd, int mask) {
  if (fd >= data_->set_size) {
    return -1;
  }

  FileEvent* fe = &data_->file_events[fd];

  if (fe->mask == EventType::EV_NONE) {
    return 0;
  }

  if (fd == data_->max_fd && fe->mask == EventType::EV_NONE) {
    int i;
    for (i = data_->max_fd - 1; i >= 0; i--) {
      if (data_->file_events[i].mask != EventType::EV_NONE) {
        break;
      }
    }

    data_->max_fd = i;
  }

  return EventLoopImpl::RemoveEvent(data_, fd, mask);
}

int EventLoop::ProcessEvent() {
  int events_num = EventLoopImpl::Poll(data_, -1);
  if (events_num < 0) {
    return -1;
  }

  for (int i = 0; i != events_num; i++) {
    FiredEvent* fired_event = &data_->fired_events[i];
    int mask = fired_event->mask;
    int fd = fired_event->fd;
    FileEvent* file_event = &data_->file_events[fd];

    if (file_event->mask & mask) {
      file_event->proc(this, fd, file_event->client_data, mask);
    } else {
      // ?????
    }
  }

  return events_num;
}

} //namespace event
} //namespace minotaur
