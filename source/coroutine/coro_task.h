#ifndef _MINOTAUR_CORO_TASK_H_
#define _MINOTAUR_CORO_TASK_H_
/**
 * @file coro_task.h
 * @author Wolfhead
 */
#include <functional>
#include <boost/function.hpp>
#include "coroutine.h"

namespace minotaur {

class CoroTask : public Coroutine {
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

} //namespace minotaur


#endif //_MINOTAUR_CORO_TASK_H_
