/**
  @file event_loop_kqueue.cpp
  @author Wolfhead
*/
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

namespace minotaur {
namespace event {

struct KqueueData {
    int kqueue_fd;
    struct kevent *events;
};

int EventLoopKqueue::Init(EventLoopData* el_data) {
  KqueueData* kq_data = new KqueueData;

  kq_data->events = new kevent[el_data->set_size];
  kq_data->kqueue_fd = kqueue();
  if (kq_data->kqueue_fd == -1) {
    return -1;
  }

  el_data->impl_data = kq_data;
  return 0;
}

int EventLoopKqueue::Destroy(EventLoopData* el_data) {
  if (!el_data || !el_data->impl_data) {
    return 0;
  }

  KqueueData* kq_data = static_cast<KqueueData*>(el_data->impl_data);
  if (kq_data->kqueue_fd) {
    close(kq_data->kqueue_fd);
  }

  if (kq_data->events) {
    delete [] kq_data->events;
  }

  delete kq_data;

  return 0;
}

int EventLoopKqueue::AddEvent(EventLoopData* el_data, int fd, int mask) {
  KqueueData* kq_data = static_cast<KqueueData*>(el_data->impl_data);
  struct kevent ke;

  if (mask & EventType::EV_READ) {
    EV_SET(&ke, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
    if (kevent(state->kqueue_fd, &ke, 1, NULL, 0, NULL) == -1) return -1;
  }
  if (mask & EventType::EV_WRITE) {
    EV_SET(&ke, fd, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
    if (kevent(state->kqueue_fd, &ke, 1, NULL, 0, NULL) == -1) return -1;
  }

  return 0;
}

int EventLoopKqueue::RemoveEvent(EventLoopData* el_data, int fd, int mask) {
  KqueueData* kq_data = static_cast<KqueueData*>(el_data->impl_data);
  struct kevent ke;

  if (mask & EventType::EV_READ) {
    EV_SET(&ke, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    if (kevent(state->kqueue_fd, &ke, 1, NULL, 0, NULL) == -1) return -1;
  }
  if (mask & EventType::EV_WRITE) {
    EV_SET(&ke, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
    if (kevent(state->kqueue_fd, &ke, 1, NULL, 0, NULL) == -1) return -1;
  }

  return 0;
}

static int aeApiPoll(EventLoopData* el_data, int timeout) {
  KqueueData* kq_data = static_cast<KqueueData*>(el_data->impl_data);
  int ret = 0;
  if (timeout <= 0) {
    ret = kevent(
            kq_data->kqueue_fd, 
            NULL, 
            0, 
            kq_data->events, 
            el_data->set_size,
            &timeout);
  } else {
    ret = kevent(
            kq_data->kqueue_fd, 
            NULL, 
            0, 
            kq_data->events, 
            el_data->set_size,
            NULL);
  }

  if (ret > 0) {
    for(int i = 0; i != ret; i++) {
      int mask = 0;
      struct kevent *e = kq_data->events + i;
      
      if (e->filter == EVFILT_READ) mask |= EventType::EV_READ;
      if (e->filter == EVFILT_WRITE) mask |= EventType::EV_WRITE;
      eventLoop->fired[i].fd = e->ident; 
      eventLoop->fired[i].mask = mask;
    }
  }

  return ret;
}

const char* EventLoopEpoll::GetImplement() const {
  return "kqueue";
}

} //namespace event
} //namespace minotaur
