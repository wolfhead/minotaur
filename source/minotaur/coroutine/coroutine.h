#ifndef _MINOTAUR_COROUTINE_H_
#define _MINOTAUR_COROUTINE_H_
/**
 * @file coroutine.h
 * @author Wolfhead
 */
#include <stdint.h>
#include <iostream>
#include "../3rd-party/libcoro/coro.h"
#include "../common/logger.h"
#include "coro_context.h"

namespace ade {

class CoroutineFactory;

class Coroutine : public coro_context {
 public:
  enum {
    kStatusWaiting = 0,
    kStatusRunning = 1,
    kStatusScheduled = 2,
  };

  Coroutine()
      : caller_(NULL)
      , coro_factory_(NULL)
      , coro_id_(0)
      , stack_size_(0)
      , status_(kStatusWaiting)
      , online_debug_(false) {
  };

  virtual ~Coroutine() {
  }

  inline void SetCoroutineId(uint64_t coro_id) {coro_id_ = coro_id;}
  inline uint64_t GetCoroutineId() const {return coro_id_;}

  inline void SetCaller(Coroutine* coro) {caller_ = coro;}
  inline Coroutine* GetCaller() const {return caller_;}

  inline void SetStackSize(uint32_t stack_size) {stack_size_ = stack_size;}
  inline uint32_t GetStackSize() const {return stack_size_;} 

  inline void SetCoroFactory(CoroutineFactory* factory) {coro_factory_ = factory;}
  inline CoroutineFactory* GetCoroutineFactory() const {return coro_factory_;}

  inline void SetNext(Coroutine* coro) {next_ = coro;}
  inline Coroutine* GetNext() const {return next_;}

  void SetStatus(uint8_t status) {status_ = status;}
  inline uint8_t GetStatus() const {return status_;}

  inline void Init() {
    coro_create(this, &Coroutine::Process, this, 
        (uint8_t*)(((uint64_t)((uint8_t*)(this + 1) + 15) >> 4) << 4), GetStackSize());
  }

  inline void Transfer(Coroutine* coro) {
    this->SetStatus(kStatusWaiting);
    coro->SetStatus(kStatusRunning);

    coro->SetCaller(this);
    CoroutineContext::SetCoroutine(coro);
    MINOTAUR_SET_ONLINE_DEBUG(coro->online_debug_);
    coro_transfer(this, coro);  
  }

  inline void Yield() {
    GetCaller()->SetStatus(kStatusRunning);
    this->SetStatus(kStatusWaiting);

    MINOTAUR_SET_ONLINE_DEBUG(false);
    coro_transfer(this, GetCaller());
  }
  
  inline void SetOnlineDebug(bool set) {
#ifndef NO_ONLINE_DEBUG
    online_debug_ = set;
#endif
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
  uint8_t status_;
#ifndef NO_ONLINE_DEBUG
  bool online_debug_;
#endif

 private:
  static void Process(void*); 
};

} //namespace ade

#endif //_MINOTAUR_COROUTINE_H_
