#ifndef _MINOTAUR_CORO_FACTORY_H_
#define _MINOTAUR_CORO_FACTORY_H_
/**
 * @file coro_factory.h
 * @author Wolfhead
 */
#include "coroutine.h"
#include "coro_bootstrap.h"
#include "../lockfree/freelist.hpp"

namespace ade {

class CoroutineFactory {
 public:
  CoroutineFactory(uint32_t stack_size);

  uint32_t GetStackSize() {return stack_size_;}

  template<typename CoroType, typename... Args>
  inline CoroType* CreateCoroutine(const Args&... args) {
    uint64_t coro_id = 0;
    CoroType* coroutine = freelist_.alloc_with<CoroType>(&coro_id, args...);
    coroutine->SetCoroutineId(coro_id);
    coroutine->SetCoroFactory(this);
    coroutine->SetStackSize(GetStackSize());
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

class ThreadLocalCorotineFactory {
 public:
  static void GlobalInit(uint32_t stack_size) {
    stack_size_ = stack_size;
  }

  inline static CoroutineFactory* Instance() {
    static thread_local CoroutineFactory factory_(stack_size_);
    return &factory_;
  }

 private:
  static uint32_t stack_size_;
};

} //namespace ade

#endif //_MINOTAUR_CORO_FACTORY_H_
