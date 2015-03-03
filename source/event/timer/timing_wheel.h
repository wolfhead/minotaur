#ifndef _MINOTAUR_TIMING_WHEEL_H_
#define _MINOTAUR_TIMING_WHEEL_H_
/**
 * @file timing_wheel.h
 * @author Wolfhead
 */
#include <iostream>
#include "../../queue/ring_buffer.h"

namespace minotaur {

template<typename T>
class TimingWheel {
 public:
  struct TimingNode {
    TimingNode* next;
    uint32_t ticks;
    T data;
  };

  typedef queue::RingBuffer<TimingNode*> Slot;

  TimingWheel(uint32_t slot, TimingWheel<T>* next = NULL)
      : current_slot_(0)
      , slots_(slot)
      , next_shift_(FastLog2(slot)) 
      , self_shift_(0)
      , mask_(slot - 1)
      , next_(NULL){
    for (uint64_t i = 0; i != slots_.Size(); ++i) {
      slots_.At(i) = NULL;
    }
    BindNext(next);
  }

  void BindNext(TimingWheel<T>* next) {
    if (!next) return;
    next_ = next;
    next_->self_shift_ = self_shift_ + next_shift_;
  }

  void AddNode(uint32_t ticks, TimingNode* node) {
    if (ticks < slots_.Size()) {
      slots_.At(current_slot_ + ticks) = 
        LinkNode(slots_.At(current_slot_ + ticks), node);
    } else {
      if (next_) {
        next_->AddNode((ticks - slots_.Size()) >> next_shift_, node);
      } else {
        assert(!"TimingWheel overflow");
      }
    }
  }

  TimingNode* Advance(uint32_t ticks) {
    TimingNode* head = NULL;
    TimingNode* tail = NULL;
    TimingNode* slot_head = NULL;

    uint64_t target_slot = current_slot_ + ticks;

    while (current_slot_ != target_slot) {
      slot_head = slots_.At(current_slot_);

      if (slot_head) {
        slots_.At(current_slot_) = NULL;
        if (tail != NULL) {
          tail->next = slot_head;
          tail = GetTail(slot_head);
        } else {
          head = slot_head;
          tail = GetTail(slot_head);
        }
      }

      ++current_slot_;
      if (slots_.Index(current_slot_) == 0) {
        PullNext();
      }
    }
    return head;
  }

  void Reset() {
    current_slot_ = 0;
    if (next_) next_->Reset();
  }

 private:

  static uint8_t FastLog2(uint32_t i) {
    uint8_t r = 0;
    while (i >>= 1) {r++;}
    return r;
  }

  static TimingNode* GetTail(TimingNode* head) {
    TimingNode* tail = head;
    while (tail && tail->next) {
      tail = tail->next;
    }
    return tail;
  }

  static TimingNode* LinkNode(TimingNode* old_head, TimingNode* new_head) {
    new_head->next = old_head;
    return new_head;
  }

  void PullNext() {
    if (!next_) return;
    TimingNode* head = next_->Advance(1);
    while (head) {
      TimingNode* next = head->next;
      AddNode(((head->ticks >> self_shift_) & mask_), head);
      head = next;
    }
  }

  uint64_t current_slot_;
  Slot slots_; 
  uint8_t next_shift_;
  uint8_t self_shift_;
  uint32_t mask_;
  TimingWheel<T>* next_;
};

} //namespace minotaur


#endif //_MINOTAUR_TIMING_WHEEL_H_
