#ifndef _MINOTAUR_EVENT_LOOP_IMPL_H_
#define _MINOTAUR_EVENT_LOOP_IMPL_H_
/**
  @file event_loop_impl.h
  @author Wolfhead <wolfhead87@gmail.com>
*/

#include "event_loop_epoll.h"

namespace minotaur {
namespace event {

typedef EventLoopEpoll EventLoopImpl;

} //namespace event
} //namespace minotaur

#endif // _MINOTAUR_EVENT_LOOP_EPOLL_H_
