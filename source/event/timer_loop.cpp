/**
  @file timer_loop.cpp
  @author Wolfhead
*/
#include "timer_loop.h"
#include <stdlib.h>
#include <sys/time.h>
#include "../lockfree/lf_util.h"

namespace minotaur {
namespace event {

TimerSlot::TimerSlot() 
    : head_(NULL) {

}

TimerSlot::~TimerSlot() {

}

void TimerSlot::PushEvent(TimerEvent* event) {
  do {
    event->next = head_;
  } while (!LF_CAS(&head_, event->next, &event));
}

bool TimerSlot::PopEvent(TimerEvent** event) {
  do {
    *event = head_;
    if (*event == NULL) {
      return false;
    }
  } while (!LF_CAS(&head_, *event, (*event)->next));
  return true;
}

TimerWheel::TimerWheel()
    : wheel_slots_(NULL)
    , event_pool_(NULL) {

}

TimerWheel::~TimerWheel() {
  Destroy();
}

int TimerWheel::Init(int wheel_size, int interval, int set_size) {
  size_ = RoundUpSize(wheel_size);
  interval_usec_ = interval;
  wheel_slots_ = new TimerSlot[size_];
  event_pool_ = new lockfree::LFStaticListPool<TimerEvent>(set_size);

  // set timer id
  TimerEvent* p;
  for (int i = 0; i != set_size; ++i) {
    if (event_pool_->At(i, &p)) {
      p->id = i;
    } else {
      return -1;
    }
  }

  return 0;
}

int TimerWheel::Destroy() {
  if (wheel_slots_) {
    delete [] wheel_slots_;
    wheel_slots_ = NULL;
  }

  if (event_pool_) {
    delete event_pool_;
    event_pool_ = NULL;
  }

  return 0;
}

int TimerWheel::AddEvent(
    int64_t when_sec,
    int64_t when_usec,
    TimerProc* proc,
    void* client_data) {
  if (!proc) {
    return -1;
  }

  TimerEvent* p = NULL;
  if (!event_pool_->Alloc(&p)) {
    return -1;
  }

  p->when_sec = when_sec;
  p->when_usec = when_usec;
  p->proc = proc;
  p->client_data = client_data;

  pending_slot_.PushEvent(p);
  return 0;
}

// after we remove the timer, the timer event is not freed immediately 
// it is still waiting for trigger in the wheel slot, then it is freed
int TimerWheel::RemoveEvent(int id) {
  TimerEvent* p = NULL;
  if (!event_pool_->At(id, &p)) {
    return -1;
  }

  void* client_data = p->client_data;
  TimerProc* proc = p->proc;
  if (LF_CAS(&p->proc, p->proc, NULL)) {
    // we now hold this timer
    proc(this, id, client_data);
    return 0;
  } else {
    // someone else cancel it before us
    return -1;
  }
}

int TimerWheel::ProcessEvent() {
  // process the pending_slot
  if (0 != ProcessPendingSlot(&pending_slot_)) {
    return -1;
  }

  int64_t now_sec;
  int64_t now_usec;
  GetCurrentTime(&now_sec, &now_usec);

  if (now_sec < current_sec_ || (now_sec == current_sec_ && now_usec < current_usec_)) {
    // timer skew
    // we will delay the timer
    return 0;
  }

  int64_t delta = (now_sec - current_sec_) * 1000000 + (now_usec - current_usec_);
  int step = delta / interval_usec_;

  while (step--) {
    current_index_ = GetIndex(current_index_ + 1);
    AddMicroSecond(&current_sec_, &current_usec_, interval_usec_);
    ProcessWheelSlot(&wheel_slots_[current_index_]);
  }

  return 0;
}

int TimerWheel::ProcessPendingSlot(TimerSlot* pending_slot) {
  TimerEvent* event = NULL;
  while (pending_slot->PopEvent(&event)) {
    if (!event->proc) { // already been removed
      event_pool_->Free(event);
      continue;
    }

    TimerSlot* slot = GetSlot(event->when_sec, event->when_usec);
    if (!slot) {
      // TODO  we should put it back
      continue;
    } else {
      slot->PushEvent(event);
    }
  }

  return 0;
}

int TimerWheel::ProcessWheelSlot(TimerSlot* slot) {
  TimerEvent* event = NULL;
  while (slot->PopEvent(&event)) {
    FireTimerEvent(event);
  }
  return 0;
}

uint32_t TimerWheel::RoundUpSize(uint32_t size) const {
  uint32_t mask = 0x80000000;
  for (;mask != 0; mask = mask >> 1) {
    if (size & mask) {
      break;
    }
  }

  if (size & ~mask) {
    return mask << 1;
  } else {
    return mask;
  }
}

void TimerWheel::GetCurrentTime(int64_t* sec, int64_t* usec) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  *sec = tv.tv_sec;
  *usec = tv.tv_usec;
}

void TimerWheel::AddMicroSecond(int64_t* sec, int64_t* usec, int64_t add_usec) {
  *usec += add_usec;
  *sec += (*usec / 1000000);
  *usec = *usec % 1000000;
}

TimerSlot* TimerWheel::GetSlot(int64_t when_sec, int64_t when_usec) {
  int64_t delta = (when_sec - current_sec_) * 1000000 + (when_usec - current_usec_);
  delta = delta < 0 ? 0 : delta;

  int32_t step = (delta / interval_usec_) + 1;
  if (step >= size_) {
    return NULL;
  }

  return &wheel_slots_[GetIndex(current_index_ + step)];
}

int TimerWheel::FireTimerEvent(TimerEvent* event) {
  int id = event->id;
  TimerProc* proc = event->proc;
  void* client_data = event->client_data;

  if (LF_CAS(&event->proc, proc, NULL)) {
    if (proc) {
      proc(this, id, client_data);
    }
  }

  event_pool_->Free(event);
  return 0;
}

} //namespace event
} //namespace minotaur
