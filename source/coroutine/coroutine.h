#ifndef _MINOTAUR_COROUTINE_H_
#define _MINOTAUR_COROUTINE_H_
/**
 * @file coroutine.h
 * @author Wolfhead
 */
#include <stdint.h>
#include <iostream>
#include "../3rd-party/libcoro/coro.h"
#include "../common/thread_id.h"
#include "coro_context.h"

namespace minotaur {

class CoroutineFactory;

class Coroutine : public coro_context {
 public:

  void SetCoroutineId(uint64_t coro_id) {coro_id_ = coro_id;}
  uint64_t GetCoroutineId() const {return coro_id_;}

  void SetCaller(Coroutine* coro) {caller_ = coro;}
  Coroutine* GetCaller() const {return caller_;}

  void SetStackSize(uint32_t stack_size) {stack_size_ = stack_size;}
  uint32_t GetStackSize() {return stack_size_;} 

  void SetCoroFactory(CoroutineFactory* factory) {coro_factory_ = factory;}
  CoroutineFactory* GetCoroutineFactory() {return coro_factory_;}

  void SetNext(Coroutine* coro) {next_ = coro;}
  inline Coroutine* GetNext() {return next_;}

  inline void Init() {
#ifndef MINOTAUR_CORO_THREAD_CHECK
    thread_id_ = ThreadId::Get();
#endif
    coro_create(this, &Coroutine::Process, this, 
        this + 1, GetStackSize());
  }

  inline void Transfer(Coroutine* coro) {
    coro->SetCaller(this);
    CoroutineContext::SetCoroutine(coro);
    coro_transfer(this, coro);  
  }

  inline void Yield() {
    coro_transfer(this, GetCaller());
  }
  
  void Destroy();

  // next_ help CoroScheduler maintain a coroutine linked list
  Coroutine* next_;
 protected:
  virtual void Run();

  Coroutine* caller_;
  CoroutineFactory* coro_factory_;
  uint64_t coro_id_;
  uint32_t stack_size_;

#ifdef MINOTAUR_CORO_THREAD_CHECK
  uint32_t thread_id_;
#endif  

 private:
  static void Process(void*); 
};

} //namespace minotaur

#endif //_MINOTAUR_COROUTINE_H_
