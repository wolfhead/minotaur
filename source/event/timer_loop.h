#ifndef _MINOTAUR_TIMER_LOOP_H_
#define _MINOTAUR_TIMER_LOOP_H_

#include <stdint.h>
#include "../lockfree/lf_static_list_pool.h"

namespace minotaur {
namespace event {

class TimerWheel;
typedef void TimerProc(TimerWheel *timer_wheel, int id, void *client_data);


struct TimerEvent {
  int id;
  int64_t when_sec;  /* seconds */
  int64_t when_usec; /* microseconds */
  TimerProc* proc;
  void* client_data;
  TimerEvent* next;
};

class TimerSlot {
 public:
  TimerSlot();

  ~TimerSlot();

  void PushEvent(TimerEvent* event);
  bool PopEvent(TimerEvent** event);

 private:

  TimerEvent* head_;
};





class TimerWheel {
 public:
  TimerWheel();

  ~TimerWheel();

  int Init(int wheel_size, int interval, int set_size);
  int Destroy();

  int AddEvent(int64_t when_sec, int64_t when_usec, TimerProc* proc, void* client_data);

  int RemoveEvent(int id);
  int ProcessEvent();

 private:
  uint32_t RoundUpSize(uint32_t size) const;
  inline int GetIndex(int index) const {
    return index & (size_ - 1);
  }

  void GetCurrentTime(int64_t* sec, int64_t* usec);
  void AddMicroSecond(int64_t* sec, int64_t* usec, int64_t add_usec);

  TimerSlot* GetSlot(int64_t when_sec, int64_t when_usec);
  int LinkSlot(TimerSlot* slot, TimerEvent* event);

  int ProcessPendingSlot(TimerSlot* slot);
  int ProcessWheelSlot(TimerSlot* slot);

  int ValidateTimerEvent(TimerEvent* event);
  int FireTimerEvent(TimerEvent* event);

  TimerSlot* wheel_slots_;
  TimerSlot  pending_slot_;

  int size_;
  int64_t interval_usec_;
  int64_t current_sec_;
  int64_t current_usec_;
  int     current_index_;

  lockfree::LFStaticListPool<TimerEvent>* event_pool_;
};

} //namespace event
} //namespace minotaur

#endif // _MINOTAUR_TIMER_LOOP_H_
