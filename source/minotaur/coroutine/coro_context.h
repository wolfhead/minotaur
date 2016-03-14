#ifndef _MINOTAUR_CORO_CONTEXT_H_
#define _MINOTAUR_CORO_CONTEXT_H_
/**
 * @file coro_context.h
 * @author Wolfhead
 */
#include "../event/timer/timer.h"
#include <unistd.h>

namespace ade {

class Coroutine;
class CoroScheduler;
class CoroutineFactory;
class IOService;
class ServiceHandler;

struct CoroutineContext {
 public:
  typedef event::Timer<uint64_t> Timer;

  static CoroutineContext& Instance() {
    static thread_local CoroutineContext context_;
    return context_;
  }

  static void Init(
      CoroutineFactory* factory,
      Timer* timer,
      IOService* io_service,
      ServiceHandler* handler) {
    Instance().factory_ = factory; 
    Instance().timer_ = timer;
    Instance().io_service_ = io_service;
    Instance().handler_ = handler;
  }

  inline static CoroutineFactory* GetFactory() {
    return Instance().factory_;
  }

  inline static Timer* GetTimer() {
    return Instance().timer_;
  }

  inline static IOService* GetIOService() {
    return Instance().io_service_;
  }

  inline static ServiceHandler* GetServiceHandler() {
    return Instance().handler_;
  }

  inline static void SetCoroutine(Coroutine* coroutine) {
    Instance().coroutine_ = coroutine;
  }
  inline static Coroutine* GetCoroutine() {
    return Instance().coroutine_;
  }

  inline static void SetScheduler(CoroScheduler* scheduler) {
    Instance().scheduler_ = scheduler;
  }
  inline static CoroScheduler* GetScheduler() {
    return Instance().scheduler_;
  }

  inline static void Dump(std::ostream& os) {
    os << "io_service:" << Instance().io_service_
       << ", factory:" << Instance().factory_
       << ", timer:" << Instance().timer_
       << ", handler:" << Instance().handler_
       << ", scheduler:" << Instance().scheduler_
       << ", coroutine:" << Instance().coroutine_
       << std::endl;
  }

 private:
  CoroutineContext() 
      : coroutine_(NULL)
      , scheduler_(NULL)
      , factory_(NULL)
      , timer_(NULL) 
      , io_service_(NULL) 
      , handler_(NULL) {
  }

  Coroutine* coroutine_;
  CoroScheduler* scheduler_;
  CoroutineFactory* factory_;
  Timer* timer_;
  IOService* io_service_;
  ServiceHandler* handler_;
};

} //namespace ade

#endif // _MINOTAUR_CORO_CONTEXT_H_
