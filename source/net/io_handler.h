#ifndef _MINOTAUR_NET_IO_HANDLER_H_
#define _MINOTAUR_NET_IO_HANDLER_H_

#include "../event/timer/timer.h"
#include "../common/logger.h"
#include "../stage.h"
#include "../handler_skeleton.h"
#include "io_message.h"

namespace minotaur {

class IOService;
class IOHandler;

class IOHandlerFactory {
 public:
  typedef IOHandler Handler;
  typedef Stage<IOHandlerFactory> StageType;

  IOHandlerFactory(IOService* io_service);

  IOHandler* Create(StageType* stage);
 private:
  IOService* io_service_;
};


class IOHandler : public HandlerSkeleton {
 public:
  typedef IOHandler self;
  typedef Stage<IOHandlerFactory> StageType;
  typedef EventMessage MessageType;
  typedef std::function<void()> TimerFunctor;
  typedef event::Timer<TimerFunctor> Timer;

  static const bool share_handler = false;
  static const bool share_queue = false;

  static uint32_t HashMessage(
      const EventMessage& message, 
      uint32_t worker_count);

  IOHandler(IOService* service, StageType* stage);

  void SetStage(StageType* stage) {stage_ = stage;}

  void Handle(const EventMessage& message);

  IOService* GetIOService() {return io_service_;}

  void OnLoopStart();

  void OnPerLoop();

  void OnIdle();

  static IOHandler* GetCurrentIOHandler() {
    return current_io_handler_;
  }

  uint64_t StartTimer(uint32_t millisecond, const TimerFunctor& functor) {
    return timer_.AddTimer(millisecond, functor);
  }

  void CancelTimer(uint64_t timer_id) {
    timer_.CancelTimer(timer_id);
  }

 private:
  LOGGER_CLASS_DECL(logger);

  void HandleIOReadEvent(const EventMessage& message);

  void HandleIOWriteEvent(const EventMessage& message);

  void HandleIOCloseEvent(const EventMessage& message);

  void HandleIOMessageEvent(const EventMessage& message);

  IOService* io_service_;
  StageType* stage_;
  Timer timer_;
  static thread_local IOHandler* current_io_handler_;
};


} // namespace minotaur

#endif // _MINOTAUR_NET_IO_HANDLER_H_
