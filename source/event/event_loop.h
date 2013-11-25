#ifndef _MINOTAUR_EVENT_LOOP_H_
#define _MINOTAUR_EVENT_LOOP_H_
/*
  @file event_loop.h
  @author Wolfhead <wolfhead@gmail.com>
*/
#include <stdint.h>
#include "event_loop_data.h"

namespace minotaur {
namespace event {

class EventLoop {
 public:
  EventLoop();
  ~EventLoop();

  int Init(int set_size);
  int Destroy();
  int Stop();

  int AddEvent(int fd, int mask, FileEventProc* proc, void* client_data);
  int RemoveEvent(int fd, int mask);
  int ProcessEvent();

  const char * GetImplement() const;

 private:
  EventLoop(const EventLoop&);
  EventLoop& operator = (const EventLoop&);

  EventLoopData* data_;
};

} //namespace event
} //namespace minotaur

#endif // _MINOTAUR_EVENT_LOOP_H_
