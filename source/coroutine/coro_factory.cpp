/**
 * @file coro_factory.h
 * @author Wolfhead
 */
#include "coro_factory.h" 

namespace minotaur {

namespace {
struct {
  union {
    char coroutine_1[sizeof(Coroutine)];
    char coroutine_2[sizeof(CoroBootstrap)];
    char coroutine_3[sizeof(CoroTask)];
  };
  uint64_t padding;
} coroutine_size;
} //namespace

CoroutineFactory::CoroutineFactory(uint32_t stack_size) 
    : stack_size_(stack_size) 
    , freelist_(4096, stack_size + sizeof(coroutine_size)) {
}

} //namespace minotaur

