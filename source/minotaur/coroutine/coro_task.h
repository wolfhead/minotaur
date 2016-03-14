#ifndef _MINOTAUR_CORO_TASK_H_
#define _MINOTAUR_CORO_TASK_H_
/**
 * @file coro_task.h
 * @author Wolfhead
 */
#include <functional>
#include <boost/function.hpp>
#include "coroutine.h"
#include "coro_actor.h"

namespace ade {

class CoroTask : public CoroActor {
 public:
  typedef boost::function<void()> TaskType;

  CoroTask(const TaskType& task) : task_(task) {}
  void SetTask(const TaskType& task) {task_ = task;}

 protected:
  virtual void Run() {
    task_();
  }

 private:
  TaskType task_;
};

} //namespace ade


#endif //_MINOTAUR_CORO_TASK_H_
