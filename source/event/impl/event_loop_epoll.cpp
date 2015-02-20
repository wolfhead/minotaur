/**
  @file event_loop_epoll.cpp
  @author Wolfhead
*/
#include "event_loop_epoll.h"
#include <unistd.h>
#include <sys/epoll.h>
#include "../event_loop_data.h"
#include "../../common/system_error.h"

namespace minotaur { namespace event {

LOGGER_CLASS_IMPL_NAME(logger, EventLoopEpoll, "event.EventLoopEpoll");

struct EpollData {
  int epoll_fd;
  struct epoll_event *events;
};

int EventLoopEpoll::Init(EventLoopData* el_data) {
  if (!el_data || !el_data->init) {
    return -1;
  }

  EpollData* ep_data = new EpollData;
  ep_data->events = new epoll_event[el_data->fd_size];
  ep_data->epoll_fd = epoll_create(1024);
  if (ep_data->epoll_fd == -1) {
    LOG_ERROR(logger, "EventLoopEpoll::Init fail with error:" 
        << SystemError::FormatMessage());
    return -1;
  }

  el_data->impl_data = ep_data;
  return 0;
}


int EventLoopEpoll::Destroy(EventLoopData* el_data) {
  if (!el_data || !el_data->impl_data) {
    return 0;
  }

  EpollData* ep_data = static_cast<EpollData*>(el_data->impl_data);
  if (ep_data->epoll_fd) {
    close(ep_data->epoll_fd);
  }

  if (ep_data->events) {
    delete [] ep_data->events;
  }

  delete ep_data;

  return 0;
}

int EventLoopEpoll::AddEvent(EventLoopData* el_data, int fd, uint32_t mask) {
  EpollData* ep_data = static_cast<EpollData*>(el_data->impl_data);

  struct epoll_event ee;

  int op = el_data->fd_events[fd].mask == EventType::EV_NONE ?
           EPOLL_CTL_ADD : EPOLL_CTL_ADD;

  ee.events = 0;
  mask |= el_data->fd_events[fd].mask; /* Merge old events */
  mask |= EventType::EV_CLOSE;
  if (mask & EventType::EV_READ) ee.events |= EPOLLIN;
  if (mask & EventType::EV_WRITE) ee.events |= EPOLLOUT;

  ee.events |= EPOLLET;
  ee.events |= EPOLLRDHUP;

  ee.data.u64 = 0; /* avoid valgrind warning */
  ee.data.fd = fd;

  if (epoll_ctl(ep_data->epoll_fd, op, fd, &ee) == -1 
      && SystemError::Get() != EEXIST) {
    LOG_ERROR(logger, "EventLoopEpoll::AddEvent fail with"
        << ", fd:" << fd
        << ", epoll_fd:" << ep_data->epoll_fd
        << ", mask:" << mask
        << ", error:" << SystemError::FormatMessage());
    return -1;
  }

  el_data->fd_events[fd].mask = mask;
  return 0;
}

int EventLoopEpoll::RemoveEvent(EventLoopData* el_data, int fd, uint32_t del_mask) {
  EpollData* ep_data = static_cast<EpollData*>(el_data->impl_data);
  struct epoll_event ee;
  uint32_t mask = el_data->fd_events[fd].mask & (~del_mask);

  ee.events = 0;
  if (mask & EventType::EV_READ) ee.events |= EPOLLIN;
  if (mask & EventType::EV_WRITE) ee.events |= EPOLLOUT;
  ee.data.u64 = 0; /* avoid valgrind warning */
  ee.data.fd = fd;

  int op = el_data->fd_events[fd].mask == EventType::EV_NONE ?
           EPOLL_CTL_ADD : EPOLL_CTL_ADD;

  if (op == EPOLL_CTL_ADD) {
    ee.events |= EPOLLET;
    ee.events |= EPOLLRDHUP;
  }

  if (epoll_ctl(ep_data->epoll_fd, op, fd, &ee) == -1
      && SystemError::Get() != EEXIST) {
    LOG_ERROR(logger, "EventLoopEpoll::RemoveEvent fail with error:" 
        << SystemError::FormatMessage()
        << ", fd:" << fd
        << ", mask:" << mask);
    return -1;
  }

  return 0;
}

int EventLoopEpoll::DeleteEvent(EventLoopData* el_data, int fd) {
  EpollData* ep_data = static_cast<EpollData*>(el_data->impl_data);
  struct epoll_event ee;
  el_data->fd_events[fd].mask = 0;

  if (epoll_ctl(ep_data->epoll_fd, EPOLL_CTL_DEL, fd, &ee) == -1
      && SystemError::Get() != EEXIST) {
    LOG_ERROR(logger, "EventLoopEpoll::DeleteEvent fail with error:" 
        << SystemError::FormatMessage()
        << ", fd:" << fd);
    return -1;
  }

  return 0;
}

int EventLoopEpoll::Poll(EventLoopData* el_data, uint32_t timeout) {
  EpollData* ep_data = static_cast<EpollData*>(el_data->impl_data);

  int ret = epoll_wait(ep_data->epoll_fd, ep_data->events, el_data->fd_size, timeout);
  if (ret > 0) {
    int events_num = ret;
    for (int i = 0; i != events_num; i++) {
      int mask = 0;
      struct epoll_event *e = ep_data->events + i;

      if (e->events & EPOLLIN) mask |= EventType::EV_READ;
      if (e->events & EPOLLOUT) mask |= EventType::EV_WRITE;
      if (e->events & EPOLLERR) mask |= EventType::EV_CLOSE;
      if (e->events & EPOLLHUP) mask |= EventType::EV_CLOSE;
      if (e->events & EPOLLRDHUP) mask |= EventType::EV_CLOSE;

      el_data->fired_events[i].fd = e->data.fd;
      el_data->fired_events[i].mask = mask;
    }
  }

  return ret;
}

const char* EventLoopEpoll::GetImplement() {
  return "epoll";
}

} //namespace event
} //namespace minotaur
