/**
 * @file coroutine.cpp
 * @author Wolfhead
 */
#include "coroutine.h"
#include <iostream>
#include "coro_factory.h"

namespace minotaur {

__thread Coroutine* Coroutine::current_coroutine_ = NULL;

void Coroutine::Process(void* arg) {
  Coroutine* coro = (Coroutine*)arg;
  coro->Run();
  Coroutine* caller = coro->GetCaller();
  coro->Destroy();
  if (caller) {
    SetCurrent(caller);
    coro_transfer(coro, caller);
  }
}

void Coroutine::Run() {
}

void Coroutine::Destroy() {
  coro_factory_->Destroy(this);
}

} //namespace minotaur



