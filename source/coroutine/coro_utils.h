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
#include "../service/service_handler.h"

namespace minotaur { namespace coro {

template<typename T = Coroutine>
inline T* GetCoroutine(uint64_t id) {
  return (T*)CoroutineFactory::GetCoroutine(id);
}

template<typename T = Coroutine>
inline T* Current() {
  return (T*)CoroutineContext::GetCoroutine();
}

inline ServiceHandler* CurrentHandler() {
  return CoroutineContext::GetServiceHandler();
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

inline void SendMail(Coroutine* coro, ProtocolMessage* message) {
  ((CoroActor*)coro)->SendMail(message);
}

inline bool Send(const EventMessage& message) {
  return CoroutineContext::GetIOService()->GetIOStage()->Send(message);
}

inline bool Send(ProtocolMessage* message) {
  message->handler_id = CurrentHandler()->GetHandlerId(); 
  message->payload.coroutine_id = Current()->GetCoroutineId();
  return coro::Send(EventMessage(
        minotaur::MessageType::kIOMessageEvent, 
        message->descriptor_id, 
        (uint64_t)message));
}

inline ProtocolMessage* Recieve(uint32_t timeout_ms = 0) {
  return ((CoroActor*)Current())->RecieveMail(timeout_ms);
}

inline ProtocolMessage* SendRecieve(ProtocolMessage* message, uint32_t timeout_ms = 0) {
  if (!Send(message)) return NULL;
  return Recieve(timeout_ms);
}

} // namespace coro
} // namespace minotaur

#endif // _MINOTAUR_COROUTINE_UTILS_H_
