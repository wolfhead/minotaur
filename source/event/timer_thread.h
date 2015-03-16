#ifndef MINOTAUR_TIMER_THREAD_H_
#define MINOTAUR_TIMER_THREAD_H_
/**
 * @file timer_thread.h
 * @author Wolfhead
 */
#include <stdint.h>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include "timer/timer.h"
#include "../common/logger.h"

namespace minotaur { namespace event {

class TimerThread : public boost::noncopyable {
 public:
  typedef void TimerProc(bool active, void* client_data);
  typedef struct {
    TimerProc* proc;
    void* client_data;
  } TimerData;
  typedef Timer<TimerData> TimerType;
  typedef TimerType::NodeType TimerNodeType;

  TimerThread(uint32_t per_tick, uint32_t total_ticks);

  ~TimerThread();

  int Start();

  int Stop();

  inline bool IsRunning() const {
    return running_;
  }

  inline uint64_t StartTimer(uint32_t milliseconds, TimerProc proc, void* data) {
    TimerData timer_data = {.proc = proc, .client_data = data};
    return timer_.AddTimer(milliseconds, timer_data);
  }

  inline void CancelTimer(uint64_t timer_id) {
    return timer_.CancelTimer(timer_id); 
  }
 
 private:
  LOGGER_CLASS_DECL(logger);

  void Run();
  
  void ProcessTimer(TimerNodeType* list_head);

  volatile bool running_;
  TimerType timer_; 
  boost::thread* thread_;
};

} //namespace event
} //namespace minotaur

#endif // MINOTAUR_TIMER_THREAD_H_
