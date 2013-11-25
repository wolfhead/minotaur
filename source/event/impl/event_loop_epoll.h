#ifndef _MINOTAUR_EVENT_LOOP_EPOLL_H_
#define _MINOTAUR_EVENT_LOOP_EPOLL_H_
/**
  @file event_loop_epoll.h
  @author Wolfhead <wolfhead87@gmail.com>
*/

namespace minotaur {
namespace event {

struct EventLoopData;
class EventLoopEpoll {
 public:
  static int Init(EventLoopData* el_data);
  static int Destroy(EventLoopData* el_data);
  static int AddEvent(EventLoopData* el_data, int fd, int mask);
  static int RemoveEvent(EventLoopData* el_data, int fd, int mask);
  static int Poll(EventLoopData* el_data, int timeout);
};

} //namespace event
} //namespace minotaur

#endif // _MINOTAUR_EVENT_LOOP_EPOLL_H_
