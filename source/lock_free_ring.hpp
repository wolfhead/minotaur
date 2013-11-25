#ifndef LOCk_FREE_RING_HPP_
#define LOCk_FREE_RING_HPP_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

namespace minotaur {

#define CAS(a_ptr, a_oldVal, a_newVal) \
__sync_bool_compare_and_swap(a_ptr, a_oldVal, a_newVal)

template<typename T>
class LockFreeRing {
 public:
  LockFreeRing(uint32_t size) 
      : size_(RoundUpSize(size))
      , ring_(NULL) 
      , read_index_(0)
      , write_index_(0)
      , read_index_max_(0) {
    assert(size);
    ring_ = new RingItem[size_];
    memset(ring, sizeof(RingItem), 0);
  }

  ~LockFreeRing() {
    if (ring_) {
      delete [] ring_;
      ring_ = NULL;
    }
  }

  bool Push(const T& item) {
    uint32_t read_index;
    uint32_t write_index;
    uint32_t next_index;

    do {
      read_index  = read_index_;
      write_index = write_index_;
      next_index = GetIndex(write_index + 1);
      
      if (next_index == read_index) {
        //printf("push s:%d,  r:%d, w:%d, m:%d\n", size_, read_index_, write_index_, read_index_max_);
        return false;
      }
    } while (!CAS(&write_index_, write_index, next_index));

    ring_[write_index] = item;

    while (!CAS(&read_index_max_, write_index, next_index)) {
      sched_yield();
    }
    return true;
  }

  bool Pop(T& item) {
    uint32_t read_index_max;
    uint32_t read_index;
    uint32_t next_index;

    do {
      read_index = read_index_;
      read_index_max = read_index_max_;
      next_index = GetIndex(read_index + 1);

      if (read_index == read_index_max) {
        //printf("pop s:%d,  r:%d, w:%d, m:%d\n", size_, read_index_, write_index_, read_index_max_);
        return false;
      }

      item = ring_[read_index];

      if (CAS(&read_index_, read_index, next_index)) {
        return true;
      }
    } while(1);

    assert(0);
    return false;
  }

  inline uint32_t Size() {
    uint32_t write_index = write_index_;
    uint32_t read_index  = read_index_;

    if (write_index >= read_index) {
      return (write_index - read_index);
    } else {
      return (size_ + write_index - read_index);
    }
  }

 private:

  struct RingItem {
    char flag;
    T item;
  };


  LockFreeRing(const LockFreeRing<T>&);
  LockFreeRing<T>& operator=(const LockFreeRing<T>&);

  uint32_t GetIndex(uint32_t index) {
    return index & (size_ - 1);
  }

  uint32_t RoundUpSize(uint32_t size) {
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


  uint32_t size_;
  RingItem* ring_;

  volatile uint32_t read_index_;
  volatile uint32_t write_index_;
  volatile uint32_t read_index_max_;
};

} //namespace zmt

#undef CAS

#endif // LOCk_FREE_RING_HPP_
