/**
 * @file coroutine.cpp
 * @author Wolfhead
 */
#include "coroutine.h"
#include <iostream>
#include "coro_factory.h"

namespace minotaur {

void Coroutine::Process(void* arg) {
  Coroutine* coro = (Coroutine*)arg;
  coro->Run();
  Coroutine* caller = coro->GetCaller();
  coro->Destroy();
  if (caller) {
    CoroutineContext::SetCoroutine(caller);
    coro_transfer(coro, caller);
  }
}

void Coroutine::Run() {
}

void Coroutine::Destroy() {
  coro_factory_->Destroy(this);
}

} //namespace minotaur



