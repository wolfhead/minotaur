/**
 * @file coro_factory.h
 * @author Wolfhead
 */
#include "coro_factory.h" 
#include "coro_scheduler.h"
#include "coro_task.h"
#include "coro_actor.h"
#include "coro_service.h"

namespace minotaur {

namespace {
typedef struct {
  union {
    char coroutine_1[sizeof(Coroutine)];
    char coroutine_2[sizeof(CoroBootstrap)];
    char coroutine_3[sizeof(CoroTask)];
    char coroutine_4[sizeof(CoroScheduler)];
    char coroutine_5[sizeof(CoroActor)];
    char coroutine_6[sizeof(CoroService)];
  } data;
  uint64_t padding;
} coroutine_size;
} //namespace

CoroutineFactory::CoroutineFactory(uint32_t stack_size) 
    : stack_size_(stack_size) 
    , freelist_(4096, stack_size + sizeof(coroutine_size)) {
}

uint32_t ThreadLocalCorotineFactory::stack_size_ = 0;

} //namespace minotaur

