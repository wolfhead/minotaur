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
  volatile TimerProc* proc;
  void* client_data;
  volatile TimerEvent* next;
};

class TimerSlot {
 public:
  virtual ~TimerSlot() {};
  virtual void PushEvent(TimerEvent* ev) = 0;
  virtual bool PopEvent(TimerEvent** ev) = 0;
};

class PlainTimerSlot : public TimerSlot {
 public:
  class iterator;

  PlainTimerSlot() : head_(NULL) {};

  virtual void PushEvent(TimerEvent* ev) {
    ev->next = head_;
    head_ = ev;
  }

  virtual bool PopEvent(TimerEvent** ev) {
    *ev = head_;

    if (head_ != NULL) {
      head_ = const_cast<TimerEvent*>(head_->next);
      return true;
    }

    return false;
  }

  inline iterator begin() const {
    return iterator(NULL, head_);
  }

  inline iterator end() const {
    return iterator(NULL, NULL);
  }

  inline iterator Remove(const iterator& it) {
    if (it.cur() == NULL) {
      return end();
    }

    if (it.prev() != NULL) {
      it.prev()->next = it.cur()->next;
    }

    return iterator(it.prev(), const_cast<TimerEvent*>(it.prev()->next));
  }

  class iterator {
   public:
    iterator(TimerEvent* prev, TimerEvent* cur) : prev_(prev), cur_(cur) {}
    iterator(const iterator& it) : prev_(it.prev_), cur_(it.cur_) {}

    iterator& operator = (const iterator& it) {
      if (&it == this) {
        return *this;
      }
      prev_ = it.prev_;
      cur_ = it.cur_;
      return *this;
    }

    bool operator == (const iterator& it) const {return cur_ == it.cur_;}
    bool operator != (const iterator& it) const {return cur_ != it.cur_;}

    iterator& operator++() {
      prev_ = cur_;
      if (cur_) {cur_ = const_cast<TimerEvent*>(cur_->next);}
      return *this;
    }

    iterator operator++(int) {
      iterator it(*this);
      ++(*this);
      return it;
    }

    inline TimerEvent* prev() const {return prev_;}
    inline TimerEvent* cur() const {return cur_;}

    TimerEvent* operator* () const {return cur_;}
    TimerEvent* operator-> () const {return cur_;}

   private:
    TimerEvent* prev_;
    TimerEvent* cur_;
  };

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
    ev->next = const_cast<TimerEvent*>(head_);
    head_ = ev;
  }

  virtual bool PopEvent(TimerEvent** ev) {
    boost::mutex::scoped_lock scope_lock(mutex_);
    *ev = const_cast<TimerEvent*>(head_);

    if (head_ != NULL) {
      head_ = head_->next;
      return true;
    }

    return false;
  }


 private:
  volatile TimerEvent* head_;
  boost::mutex mutex_;
};

class CasTimerSlot : public TimerSlot {
 public:
  CasTimerSlot() : head_(NULL) {}

  virtual void PushEvent(TimerEvent* event) {
    do {
      event->next = const_cast<TimerEvent*>(head_);
    } while (!LF_CAS(&head_, event->next, event));
  }

  virtual bool PopEvent(TimerEvent** event) {
    do {
      *event = const_cast<TimerEvent*>(head_);
      if (*event == NULL) {
        return false;
      }
    } while (!LF_CAS(&head_, *event, (*event)->next));
    return true;
  }

 private:
  volatile TimerEvent* head_;
};

} //namespace event
} //namespace minotaur

#endif // _MINOTAUR_TIMER_SLOT_H_

