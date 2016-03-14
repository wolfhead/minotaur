#ifndef _MINOTAUR_COROUTINE_SCHEDULER_H_
#define _MINOTAUR_COROUTINE_SCHEDULER_H_ 
/**
 * @file coro_scheduler.h
 * @author Wolfhead
 */
#include "coroutine.h"
#include "../queue/linked_list.h"
#include <iostream>

namespace ade {

class CoroScheduler : public Coroutine {
 public:
  CoroScheduler() {
    CoroutineContext::SetScheduler(this);
  }

  inline void Schedule(Coroutine* coro) {
    if (coro->GetStatus() == Coroutine::kStatusScheduled) return;
    coro->SetStatus(Coroutine::kStatusScheduled);
    coro->next_ = NULL;
    task_.push_back(coro);
  }

  inline void SchedulePri(Coroutine* coro) {
    if (coro->GetStatus() == Coroutine::kStatusScheduled) return;
    coro->SetStatus(Coroutine::kStatusScheduled);
    coro->next_ = NULL;
    task_.push_front(coro);
  }

 protected:
  virtual void Run() {
    Coroutine* coro = task_.front();
    while (coro) {
      task_.pop_front();
      Transfer(coro);
      coro = task_.front();
    }
  }

 private:
  LinkedList<Coroutine> task_;
};


} //namespace ade

#endif //_MINOTAUR_COROUTINE_SCHEDULER_H_
