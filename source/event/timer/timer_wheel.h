#ifndef _MINOTAUR_TIMER_WHEEL_H_
#define _MINOTAUR_TIMER_WHEEL_H_
/**
  @file timer_wheel.h
  @author Wolfhead
*/
#include "timer_slot.h"
#include "../../lockfree/lf_static_list_pool.h"

namespace minotaur {
namespace event {

class TimerWheel {
 public:
  TimerWheel();

  ~TimerWheel();

  int Init(int wheel_size, int interval_usecond, int set_size);
  int Destroy();

  int AddEvent(int64_t when_sec, int64_t when_usec, TimerProc* proc, void* client_data);

  int RemoveEvent(int id);
  int ProcessEvent();

  static void GetCurrentTime(int64_t* sec, int64_t* usec);
  static void AddMicroSecond(int64_t* sec, int64_t* usec, int64_t add_usec);

 private:
  uint32_t RoundUpSize(uint32_t size) const;
  inline int GetIndex(int index) const {
    return index & (size_ - 1);
  }


  TimerSlot* GetSlot(int64_t when_sec, int64_t when_usec);
  int LinkSlot(TimerSlot* slot, TimerEvent* event);

  int ProcessPendingSlot(TimerSlot* slot);
  int ProcessWheelSlot(TimerSlot* slot);

  int ValidateTimerEvent(TimerEvent* event);
  int FireTimerEvent(TimerEvent* event);

  PlainTimerSlot* wheel_slots_;
  CasTimerSlot* pending_slot_;

  int size_;
  int64_t interval_usec_;
  int64_t current_sec_;
  int64_t current_usec_;
  int     current_index_;

  lockfree::LFStaticListPool<TimerEvent>* event_pool_;
};

} //namespace event
} //namespace minotaur

#endif // _MINOTAUR_TIMER_WHEEL_H_
