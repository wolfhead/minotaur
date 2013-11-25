/**
  @file event_loop_epoll.cpp
  @author Wolfhead
*/
#include "event_loop_epoll.h"
#include <unistd.h>
#include <sys/epoll.h>
#include "../event_loop_data.h"

namespace minotaur {
namespace event {

struct EpollData {
  int epoll_fd;
  struct epoll_event *events;
};

int EventLoopEpoll::Init(EventLoopData* el_data) {
  EpollData* ep_data = new EpollData;

  ep_data->events = new epoll_event[el_data->set_size];
  ep_data->epoll_fd = epoll_create(1024);
  if (ep_data->epoll_fd == -1) {
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

int EventLoopEpoll::AddEvent(EventLoopData* el_data, int fd, int mask) {
  EpollData* ep_data = static_cast<EpollData*>(el_data->impl_data);

  struct epoll_event ee;

  int op = el_data->file_events[fd].mask == EventType::EV_NONE ?
           EPOLL_CTL_ADD : EPOLL_CTL_MOD;

  ee.events = 0;
  mask |= el_data->file_events[fd].mask; /* Merge old events */
  if (mask & EventType::EV_READ) ee.events |= EPOLLIN;
  if (mask & EventType::EV_WRITE) ee.events |= EPOLLOUT;
  ee.data.u64 = 0; /* avoid valgrind warning */
  ee.data.fd = fd;

  if (epoll_ctl(ep_data->epoll_fd, op, fd, &ee) == -1) {
    return -1;
  }
  return 0;
}

int EventLoopEpoll::RemoveEvent(EventLoopData* el_data, int fd, int del_mask) {
  EpollData* ep_data = static_cast<EpollData*>(el_data->impl_data);
  struct epoll_event ee;
  int mask = el_data->file_events[fd].mask & (~del_mask);

  ee.events = 0;
  if (mask & EventType::EV_READ) ee.events |= EPOLLIN;
  if (mask & EventType::EV_WRITE) ee.events |= EPOLLOUT;
  ee.data.u64 = 0; /* avoid valgrind warning */
  ee.data.fd = fd;

  int op = el_data->file_events[fd].mask == EventType::EV_NONE ?
           EPOLL_CTL_MOD : EPOLL_CTL_DEL;

  if (epoll_ctl(ep_data->epoll_fd, op, fd, &ee) == -1) {
    return -1;
  }
  return 0;
}

int EventLoopEpoll::Poll(EventLoopData* el_data, int timeout) {
  EpollData* ep_data = static_cast<EpollData*>(el_data->impl_data);

  int ret = epoll_wait(ep_data->epoll_fd, ep_data->events, el_data->set_size, timeout);
  if (ret > 0) {
    int events_num = ret;
    for (int i = 0; i != events_num; i++) {
      int mask = 0;
      struct epoll_event *e = ep_data->events + i;

      if (e->events & EPOLLIN) mask |= EventType::EV_READ;
      if (e->events & EPOLLOUT) mask |= EventType::EV_WRITE;
      if (e->events & EPOLLERR) mask |= EventType::EV_WRITE;
      if (e->events & EPOLLHUP) mask |= EventType::EV_WRITE;
      el_data->fired_events[i].fd = e->data.fd;
      el_data->fired_events[i].mask = mask;
    }
  }

  return ret;
}

} //namespace event
} //namespace minotaur
