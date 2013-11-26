#ifndef _MINOTAUR_TIMER_SLOT_H_
#define _MINOTAUR_TIMER_SLOT_H_ 
/**
  @file timer_slot.h
  @author Wolfhead
*/

#include <stdint.h>
#include <stdlib.h>
#include <boost/thread.hpp>
#include "../../lockfree/lf_util.h"

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
  virtual ~TimerSlot() {};
  virtual void PushEvent(TimerEvent* ev) = 0;
  virtual bool PopEvent(TimerEvent** ev) = 0;
};

class PlainTimerSlot : public TimerSlot {
 public:
  PlainTimerSlot() : head_(NULL) {};

  virtual void PushEvent(TimerEvent* ev) {
    ev->next = head_;
    head_ = ev;
  }

  virtual bool PopEvent(TimerEvent** ev) {
    *ev = head_;

    if (head_ != NULL) {
      head_ = head_->next;
      return true;
    }

    return false;
  }

 private:
  TimerEvent* head_;
};

/**
  MutexTimerSlot is mainly used for benchmark test
*/
class MutexTimerSlot : public TimerSlot {
 public:
  MutexTimerSlot() : head_(NULL) {}

  virtual void PushEvent(TimerEvent* ev) {
    boost::mutex::scoped_lock scope_lock(mutex_);
    ev->next = head_;
    head_ = ev;
  }

  virtual bool PopEvent(TimerEvent** ev) {
    boost::mutex::scoped_lock scope_lock(mutex_);
    *ev = head_;

    if (head_ != NULL) {
      head_ = head_->next;
      return true;
    }

    return false;
  }


 private:
  TimerEvent* head_;
  boost::mutex mutex_;
};

class CasTimerSlot : public TimerSlot {
 public:
  CasTimerSlot() : head_(NULL) {}

  virtual void PushEvent(TimerEvent* event) {
    do {
      event->next = head_;
    } while (!LF_CAS(&head_, event->next, event));
  }

  virtual bool PopEvent(TimerEvent** event) {
    do {
      *event = head_;
      if (*event == NULL) {
        return false;
      }
    } while (!LF_CAS(&head_, *event, (*event)->next));
    return true;
  }

 private:
  TimerEvent* head_;
};

} //namespace event
} //namespace minotaur

#endif // _MINOTAUR_TIMER_SLOT_H_

