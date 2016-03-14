#ifndef _MINOTAUR_EVENT_LOOP_H_
#define _MINOTAUR_EVENT_LOOP_H_
/*
  @file event_loop.h
  @author Wolfhead <wolfhead@gmail.com>
*/
#include <stdint.h>
#include "event_loop_data.h"
#include "../common/logger.h"

namespace ade {
namespace event {

class EventLoop {
 public:
  EventLoop();
  ~EventLoop();

  int Init(uint32_t fd_size);
  int Destroy();
  int Stop();

  int AddEvent(int fd, uint32_t mask, FdEventProc* proc, void* client_data);
  int RemoveEvent(int fd, uint32_t mask);
  int DeleteEvent(int fd);

  /**
    Process Event 
    @param timeout millisecond
    @return number of events processed if >= 0, system error if < 0 
  */
  int ProcessEvent(uint32_t timeout);
  void FireEvent(int fd, uint32_t mask);

  const char * GetImplement() const;

 private:
  LOGGER_CLASS_DECL(logger);

  EventLoop(const EventLoop&);
  EventLoop& operator = (const EventLoop&);

  EventLoopData data_;
};

} //namespace event
} //namespace ade

#endif // _MINOTAUR_EVENT_LOOP_H_
