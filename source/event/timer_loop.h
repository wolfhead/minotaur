#ifndef _MINOTAUR_TIMER_LOOP_H_
#define _MINOTAUR_TIMER_LOOP_H_

#include "../static_list_pool.h"

namespace minotaur {
namespace event {

typedef void TimerProc(TimerLoop *timerLoop, int id, void *client_data);


struct TimerEvent {
  int id;
  int64_t when_sec;
  int64_t when_usec;
  TimerProc* proc;
  void* client_data;
  TimerEvent* next;
};

struct TimerSlot {
  TimerEvent* head;
  int size;
};





class TimerWheel {
 public:
  TimerWheel();

  ~TimerWheel();

  int Init(int wheel_size; int interval, int set_size);
  int Destroy();

  int AddEvent(int64_t when_sec, int64_t when_msec, TimerProc* proc, void* client_data);
  int RemoveEvent(int id);

 private:
  uint32_t RoundUpSize(uint32_t size) const;
  void GetCurrentTime(int64_t* sec, int64_t* usec);


  TimerSlot* GetSlot(timeout)

  TimerSlot* slots_;
  int size_;
  int interval_usec;
  int64_t current_sec;
  int64_t current_usec;
  int     current_index;

  StaticListPool<TimerEvent>* events_;
};

} //namespace event
} //namespace minotaur

#endif // _MINOTAUR_TIMER_LOOP_H_
