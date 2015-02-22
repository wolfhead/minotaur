#ifndef _MINOTAUR_CORO_FACTORY_H_
#define _MINOTAUR_CORO_FACTORY_H_
/**
 * @file coro_factory.h
 * @author Wolfhead
 */
#include "coroutine.h"
#include "coro_bootstrap.h"
#include "coro_task.h"
#include "../lockfree/freelist.hpp"

namespace minotaur {

class CoroutineFactory {
 public:
  CoroutineFactory(uint32_t stack_size);

  uint32_t GetStackSize() {return stack_size_;}

  template<typename CoroType = Coroutine>
  inline CoroType* CreateCoroutine() {
    uint64_t coro_id = 0;
    CoroType* coroutine = freelist_.alloc_with<CoroType>(&coro_id);
    coroutine->SetCoroutineId(coro_id);
    coroutine->SetCoroFactory(this);
    coroutine->SetStackSize(GetStackSize());
    coroutine->Init();
    return coroutine;
  }

  inline CoroTask* CreateCoroutineTask(const CoroTask::TaskType& task) {
    uint64_t coro_id = 0;
    CoroTask* coroutine = freelist_.alloc_with<CoroTask>(&coro_id);
    coroutine->SetCoroutineId(coro_id);
    coroutine->SetCoroFactory(this);
    coroutine->SetStackSize(GetStackSize());
    coroutine->SetTask(task);
    coroutine->Init();
    return coroutine;
  }

  bool Destroy(Coroutine* coro) {
    return freelist_.destroy(coro);  
  }

  static Coroutine* GetCoroutine(uint64_t coro_id) {
    return Freelist::get_key(coro_id);  
  }

 private:
  typedef lockfree::freelist<Coroutine> Freelist;

  uint32_t stack_size_;
  Freelist freelist_;
};

} //namespace minotaur

#endif //_MINOTAUR_CORO_FACTORY_H_
