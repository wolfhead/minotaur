#ifndef _MINOTAUR_COROUTINE_SCHEDULER_H_
#define _MINOTAUR_COROUTINE_SCHEDULER_H_ 
/**
 * @file coro_scheduler.h
 * @author Wolfhead
 */
#include "coroutine.h"
#include "../queue/linked_list.h"
#include <iostream>

namespace minotaur {

class CoroScheduler : public Coroutine {
 public:
  CoroScheduler() {
    CoroutineContext::SetScheduler(this);
  }

  inline void Schedule(Coroutine* coro) {
    std::cout << "try schedule:" << coro->GetCoroutineId() << std::endl;
    task_.push_back(coro);
  }

  inline void SchedulePri(Coroutine* coro) {
    task_.push_front(coro);
  }

 protected:
  virtual void Run() {
    Coroutine* coro = task_.front();
    while (coro) {
      task_.pop_front();
      std::cout << "schedule:" << coro->GetCoroutineId() << std::endl;
      Transfer(coro);
      coro = task_.front();
    }
  }

 private:
  LinkedList<Coroutine> task_;
};


} //namespace minotaur

#endif //_MINOTAUR_COROUTINE_SCHEDULER_H_
