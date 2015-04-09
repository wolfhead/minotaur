#ifndef _MINOTAUR_EVENT_TIMER_H_
#define _MINOTAUR_EVENT_TIMER_H_
/**
 * @file timer.h
 * @author Wolfhead
 */
#include <stdint.h>
#include "timing_wheel.h"
#include "../../lockfree/freelist.hpp"
#include "../../common/time_util.h"

namespace minotaur { namespace event {

template<typename T>
class Timer {
 public:
  typedef TimingWheel<T> TimingWheelType;
  typedef typename TimingWheel<T>::TimingNode NodeType;

  Timer(uint32_t millisecond_per_tick, uint32_t totol_ticks);
  ~Timer();

  uint64_t AddTimer(uint32_t milliseconds, const T& data);
  void CancelTimer(uint64_t timer_id);

  NodeType* ProcessTimer();
  void DestroyTimerNode(NodeType* node);

 private:

  void BuildWheel(uint32_t total_ticks);
  void DestroyWheel();

  uint64_t last_process_time_;
  uint32_t per_tick_;
  uint32_t total_ticks_;
  TimingWheelType* wheel_;
  lockfree::freelist<NodeType> freelist_;
};


template<typename T>
Timer<T>::Timer(uint32_t millisecond_per_tick, uint32_t totol_ticks)
    : last_process_time_(Time::GetMillisecond())
    , per_tick_(millisecond_per_tick)
    , total_ticks_(totol_ticks)
    , freelist_(1024) {
  BuildWheel(total_ticks_);
}

template<typename T>
Timer<T>::~Timer() {
  NodeType* node = wheel_->Advance(total_ticks_);
  DestroyTimerNode(node);
  DestroyWheel();
}

template<typename T>
uint64_t Timer<T>::AddTimer(uint32_t milliseconds, const T& data) {
  uint64_t timer_id = 0;
  NodeType* node = freelist_.alloc(&timer_id);
  
  node->ticks = (Time::GetMillisecond() - last_process_time_ +  milliseconds) / per_tick_;
  if (node->ticks > total_ticks_) node->ticks = 1;

  node->active = true;
  node->data = data;
  wheel_->AddNode(node);
  return timer_id;
}
  
template<typename T>
void Timer<T>::CancelTimer(uint64_t timer_id) {
  NodeType* node = lockfree::freelist<NodeType>::get_key(timer_id);
  if (node) node->active = false;
  return;
}

template<typename T>
typename Timer<T>::NodeType* Timer<T>::ProcessTimer() {
  uint64_t current_process_time = Time::GetMillisecond();
  uint32_t ticks = (current_process_time - last_process_time_) / per_tick_;
  if (ticks == 0) return NULL;
  if (ticks > total_ticks_) ticks = 1;

  last_process_time_ = current_process_time;
  return wheel_->Advance(ticks);
}

template<typename T>
void Timer<T>::DestroyTimerNode(NodeType* node) {
  while (node) {
    NodeType* next = node->next;
    freelist_.destroy(node);
    node = next;
  }
}

template<typename T>
void Timer<T>::BuildWheel(uint32_t total_ticks) {
  TimingWheelType* current = wheel_ = new TimingWheelType(256);
  total_ticks >>= 8;

  while (total_ticks != 0) {
    TimingWheelType* next = new TimingWheelType(64);
    total_ticks >>= 6;

    current->BindNext(next);
    current = next;
  }
}

template<typename T>
void Timer<T>::DestroyWheel() {
  TimingWheelType* current = wheel_;
  while (current) {
    TimingWheelType* next = current->GetNext();
    delete current;
    current = next;
  }
}

} //namespace event
} //namespace minotaur

#endif // _MINOTAUR_EVENT_TIMER_H_
