#ifndef _MINOTAUR_EVENT_LOOP_DATA_H_
#define _MINOTAUR_EVENT_LOOP_DATA_H_
/**
  @file event_loop_data.h
  @author Wolfhead
*/
#include <stdint.h>

namespace minotaur {
namespace event {

class EventLoop;

struct EventType {
  enum {
    EV_NONE = 0,
    EV_READ = 1,
    EV_WRITE = 2,
    EV_CLOSE = 4,
  };
};

typedef void FdEventProc(EventLoop *eventLoop, int fd, void *clientData, uint32_t mask);

struct FdEvent {
  uint32_t      mask;
  FdEventProc*  proc;
  void*         client_data;
};

struct FiredEvent {
  uint32_t      fd;
  uint32_t      mask;
};

struct EventLoopData {
  EventLoopData();
  ~EventLoopData();

  void Init(uint32_t fd_size);
  void Destroy();

  bool          init;
  uint32_t      fd_size;
  FdEvent*      fd_events;
  FiredEvent*   fired_events;
  void*         impl_data;
};

} //namespace event
} //namespace minotaur

#endif // _MINOTAUR_EVENT_LOOP_DATA_H_
