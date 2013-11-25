#ifndef _MINOTAUR_EVENT_LOOP_IMPL_H_
#define _MINOTAUR_EVENT_LOOP_IMPL_H_
/**
  @file event_loop_impl.h
  @author Wolfhead <wolfhead87@gmail.com>
*/

#include "../../config.h"

#ifdef HAS_EPOLL
  #include "event_loop_epoll.h"
#else 
  #ifdef HAS_KQUEUE
    #include "event_loop_kqueue.h"
  #endif // HAS_KQUEUE
#endif // HAS_EPOOL


#endif // _MINOTAUR_EVENT_LOOP_EPOLL_H_
