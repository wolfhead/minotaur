#ifndef _MINOTAUR_EVENT_LOOP_EPOLL_H_
#define _MINOTAUR_EVENT_LOOP_EPOLL_H_
/**
  @file event_loop_epoll.h
  @author Wolfhead <wolfhead87@gmail.com>
*/
#include <stdint.h>
#include "../../common/logger.h"

namespace ade {
namespace event {

struct EventLoopData;
class EventLoopEpoll {
 public:
  static int Init(EventLoopData* el_data);
  static int Destroy(EventLoopData* el_data);
  static int AddEvent(EventLoopData* el_data, int fd, uint32_t mask);
  static int RemoveEvent(EventLoopData* el_data, int fd, uint32_t mask);
  static int DeleteEvent(EventLoopData* el_data, int fd);
  static int Poll(EventLoopData* el_data, uint32_t timeout);
  static const char* GetImplement();
 private:
  LOGGER_CLASS_DECL(logger);
};

typedef EventLoopEpoll EventLoopImpl;

} //namespace event
} //namespace ade

#endif // _MINOTAUR_EVENT_LOOP_EPOLL_H_
