#ifndef _MINOTAUR_TIMING_WHEEL_H_
#define _MINOTAUR_TIMING_WHEEL_H_
/**
 * @file timing_wheel.h
 * @author Wolfhead
 */
#include <iostream>
#include "../../queue/ring_buffer.h"

namespace minotaur { namespace event {

template<typename T>
class TimingWheel {
 public:
  struct TimingNode {
    TimingNode* next;
    uint32_t ticks;
    bool active;
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

  TimingWheel<T>* GetNext() {return next_;}

  void BindNext(TimingWheel<T>* next) {
    if (!next) return;
    next_ = next;
    next_->self_shift_ = self_shift_ + next_shift_;
  }

  void AddNode(TimingNode* node) {
    AddNode(node->ticks, node);
  }

  TimingNode* Advance(uint32_t ticks) {
    if (ticks == 0) {
      return NULL;
    }

    TimingNode* head = NULL;
    TimingNode* tail = NULL;
    TimingNode* slot_head = NULL;

    while (ticks--) {
      ++current_slot_;
      if (slots_.Index(current_slot_) == 0) {
        PullNext(); 
      }

      slot_head = slots_.At(current_slot_);
      slots_.At(current_slot_) = NULL;
      head = LinkList(&head, &tail, slot_head);
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

  static TimingNode* LinkList(TimingNode** head, TimingNode** tail, TimingNode* link_head) {
    if (!link_head) {
      return *head;
    }
    if (*tail == NULL) {
      *head = link_head;
      *tail = GetTail(link_head);
    } else {
      (*tail)->next = link_head;
      *tail = GetTail(link_head);
    }
    return *head;
  }

  static TimingNode* LinkNode(TimingNode* old_head, TimingNode* new_head) {
    new_head->next = old_head;
    return new_head;
  }

  void AddNode(uint32_t ticks, TimingNode* node) {
    if (ticks <= slots_.Size()) {
      uint64_t tick_slot = current_slot_ + ticks;
      slots_.At(tick_slot) = LinkNode(slots_.At(tick_slot), node);
    } else {
      if (next_) {
        // remove the remain ticks from the node
        // because every PullNext starts at offset 0
        uint32_t remain_slot = (slots_.Size() - slots_.Index(current_slot_));
        uint32_t offset = (ticks - remain_slot) & mask_;
        uint32_t loop = (ticks + slots_.Index(current_slot_)) >> next_shift_;
        node->ticks = (node->ticks & ~(mask_ << self_shift_)) | offset << self_shift_;
        next_->AddNode(loop, node);
      } else {
        assert(!"TimingWheel overflow");
      }
    }
  }

  void PullNext() {
    if (!next_) return;
    TimingNode* head = next_->Advance(1);

    while (head) {
      TimingNode* next = head->next;
      uint32_t ticks = (head->ticks >> self_shift_) & mask_;
      AddNode(ticks, head);
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

} //namespace event
} //namespace minotaur


#endif //_MINOTAUR_TIMING_WHEEL_H_
