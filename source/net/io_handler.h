#ifndef _MINOTAUR_NET_IO_HANDLER_H_
#define _MINOTAUR_NET_IO_HANDLER_H_

#include "../event/timer/timer.h"
#include "../common/logger.h"
#include "../stage.h"
#include "../handler.h"
#include "io_message.h"

namespace minotaur {

class IOService;
class IOHandler;

class IOHandler : public HandlerSkeleton {
 public:
  typedef std::function<void()> TimerFunctor;
  typedef event::Timer<TimerFunctor> Timer;

  inline static uint32_t Hash(const EventMessage& message) {
    return message.descriptor_id >> 48;
  }

  IOHandler(IOService* io_service);

  void Run(StageData* data);

  void Handle(const EventMessage& message);

  static IOHandler* GetCurrentIOHandler() {
    return current_io_handler_;
  }

  uint64_t StartTimer(uint32_t millisecond, const TimerFunctor& functor) {
    return timer_.AddTimer(millisecond, functor);
  }

  void CancelTimer(uint64_t timer_id) {
    timer_.CancelTimer(timer_id);
  }

  IOHandler* Clone();

 private:
  LOGGER_CLASS_DECL(logger);

  void ProcessTimer();

  void HandleIOReadEvent(const EventMessage& message);

  void HandleIOWriteEvent(const EventMessage& message);

  void HandleIOCloseEvent(const EventMessage& message);

  void HandleIOMessageEvent(const EventMessage& message);

  void HandleIOActiveCloseEvent(const EventMessage& message);

  void HandleIOMessageFailure(const EventMessage& message);

  Timer timer_;
  static thread_local IOHandler* current_io_handler_;
};


} // namespace minotaur

#endif // _MINOTAUR_NET_IO_HANDLER_H_
