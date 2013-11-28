#ifndef LOCk_FREE_RING_HPP_
#define LOCk_FREE_RING_HPP_

#include <stdint.h>
#include <stdlib.h>
#include <boost/atomic.hpp>

namespace zmt {

#define CAS(a_ptr, a_oldVal, a_newVal) \
__sync_bool_compare_and_swap(a_ptr, a_oldVal, a_newVal)

template<typename T>
class LockFreeRing {
 public:
  typedef uint32_t SizeType;

  LockFreeRing(SizeType size) 
      : size_(RoundUpSize(size))
      , ring_(NULL) 
      , read_index_(0)
      , write_index_(0) 
      , max_read_(0) {
    ring_ = new RingItem[size_];
    memset(ring_, sizeof(RingItem) * size_, 0);
  }

  ~LockFreeRing() {
    if (ring_) {
      delete [] ring_;
      ring_ = NULL;
    }
  }

  bool Push(const T& item) {
    uint32_t write_index;
    uint32_t next_index;
    uint32_t max_read;

    // claim a slot
    do {
      write_index = write_index_;
      next_index = GetIndex(write_index + 1);

      if (next_index == read_index_) {
        return false;
      }

    } while (!CAS(&write_index_, write_index, next_index));

    // prepare the data and commit
    ring_[write_index].item = item;
    ring_[write_index].flag = RingItem::kCommit;

    do {
      max_read = max_read_;
      if (ring_[max_read].flag != RingItem::kCommit) {
        break;
      }
      next_index = GetIndex(max_read + 1);
    } while (CAS(&max_read_, max_read, next_index));

    return true;
  }

  bool Pop(T* item) {
    uint32_t read_index;
    uint32_t next_index;
    uint32_t max_read;

    do {
      max_read = max_read_;
      read_index = read_index_;
      
      if (max_read == read_index) {
        //std::cout << "Pop_fail:" << read_index << std::endl;
        return false;
      }

      next_index = GetIndex(read_index + 1);

      *item = ring_[read_index].item;
      ring_[read_index].flag = RingItem::kFree;

      //std::cout << "Pop_try:" << read_index << std::endl;
    } while(!CAS(&read_index_, read_index, next_index));

    //std::cout << "Pop_success:" << read_index << std::endl;
    return true;
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
    enum {
      kFree = 0,
      kCommit
    };

    boost::atomic<char> flag;
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
  volatile uint32_t max_read_;
};

} //namespace zmt

#undef CAS

#endif // LOCk_FREE_RING_HPP_
