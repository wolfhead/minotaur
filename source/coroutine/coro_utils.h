#ifndef _MINOTAUR_COROUTINE_UTILS_H_
#define _MINOTAUR_COROUTINE_UTILS_H_
/**
 * @file coro_utils.h
 * @author Wolfhead
 */
#include "coro_all.h"
#include "../io_service.h"
#include "../stage.h"
#include "../net/io_handler.h"

namespace minotaur { namespace coro {

template<typename T = Coroutine>
inline T* GetCoroutine(uint64_t id) {
  return (T*)CoroutineFactory::GetCoroutine(id);
}

template<typename T = Coroutine>
inline T* Current() {
  return (T*)CoroutineContext::GetCoroutine();
}

inline CoroScheduler* Scheduler() {
  return CoroutineContext::GetScheduler();
}

inline void Transfer(Coroutine* coro) {
  Current()->Transfer(coro);
}

inline void Yield() {
  Current()->Yield();
}

inline void Schedule(Coroutine* coro) {
  Scheduler()->Schedule(coro);
}

inline void SchedulePriority(Coroutine* coro) {
  Scheduler()->SchedulePri(coro);
}

template<typename T, typename... Args>
inline T* Spawn(const Args&... args) {
  return CoroutineContext::GetFactory()->CreateCoroutine<T>(args...);
}

template<typename T, typename... Args>
inline T* SpawnAndSchedule(const Args&... args) {
  T* coro = CoroutineContext::GetFactory()->CreateCoroutine<T>(args...);
  Schedule(coro);
  return coro;
}

inline uint64_t StartTimer(uint32_t millisecond) {
  return CoroutineContext::GetTimer()->AddTimer(millisecond, Current()->GetCoroutineId());
}

inline void CancelTimer(uint64_t timer_id) {
  CoroutineContext::GetTimer()->CancelTimer(timer_id);
}

inline bool Send(const EventMessage& message) {
  return CoroutineContext::GetIOService()->GetIOStage()->Send(message);
}

inline bool Send(ProtocolMessage* message) {
  return coro::Send(EventMessage(
        minotaur::MessageType::kIOMessageEvent, 
        message->descriptor_id, 
        (uint64_t)message));
}

} // namespace coro
} // namespace minotaur

#endif // _MINOTAUR_COROUTINE_UTILS_H_
