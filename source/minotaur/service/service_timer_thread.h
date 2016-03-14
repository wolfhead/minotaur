#ifndef MINOTAUR_SERVICE_TIMER_THREAD_H
#define MINOTAUR_SERVICE_TIMER_THREAD_H
/**
 * @file service_timer_thread.h
 * @author Wolfhead
 */
#include <boost/thread.hpp>
#include <functional>
#include "../io_service.h"
#include "../common/logger.h"
#include "../common/spin_lock.h"
#include "../event/timer/timer.h"

namespace ade {

class ServiceTimerThread {
 public:
  typedef std::function<void()> TimerFunc; 
  typedef event::Timer<TimerFunc> Timer;

  ServiceTimerThread(
      IOService* io_service, 
      IOService::ServiceStage* stage);

  ~ServiceTimerThread();

  inline IOService* GetIOService() {return io_service_;}

  int Start();

  int Stop();

  uint64_t StartTimer(uint32_t ms, const TimerFunc& func) {
    lock_.lock();
    uint64_t timer_id = timer_.AddTimer(ms, func);
    lock_.unlock();
    return timer_id;
  }

  void CancelTimer(uint64_t timer_id) {
    lock_.lock();
    timer_.CancelTimer(timer_id);
    lock_.unlock();
  }

  int ScheduleTask(const TimerFunc& func);

 private:
  LOGGER_CLASS_DECL(logger);

  void Run();

  void ProcessTimer();

  IOService* io_service_;
  IOService::ServiceStage* service_stage_;

  boost::thread* thread_;
  bool running_;

  Timer timer_;
  spinlock lock_;
};

} //namesapce ade

#endif //MINOTAUR_SERVICE_TIMER_THREAD_H
