#ifndef _MINOTAUR_COROUTINE_H_
#define _MINOTAUR_COROUTINE_H_
/**
 * @file coroutine.h
 * @author Wolfhead
 */
#include <stdint.h>
#include <iostream>
#include "libcoro/coro.h"

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

  void SetCoroFactory(CoroutineFactory* factory) {
    coro_factory_ = factory;
  }
  CoroutineFactory* GetCoroutineFactory() {
    return coro_factory_;
  }

  inline void Init() {
    coro_create(this, &Coroutine::Process, this, 
        this + 1, GetStackSize());
  }

  inline void Transfer(Coroutine* coro) {
    coro->SetCaller(this);
    SetCurrent(coro);
    coro_transfer(this, coro);  
  }

  inline void Yield() {
    SetCurrent(GetCaller());
    coro_transfer(this, GetCaller());
  }
  
  inline void GetStatus() {
    std::cout << sp << std::endl;
  }

  void Destroy();

  inline static Coroutine* Current() {
    return current_coroutine_;
  };

  inline static void SetCurrent(Coroutine* coro) {
    current_coroutine_ = coro;
  }

 protected:
  virtual void Run();

 private:
  static void Process(void*); 

  Coroutine* caller_;
  CoroutineFactory* coro_factory_;
  uint64_t coro_id_;
  uint32_t stack_size_;

  static __thread Coroutine* current_coroutine_;
};



} //namespace minotaur

#endif //_MINOTAUR_COROUTINE_H_
