#ifndef _MINOTAUR_SEM_FIFO_H_
#define _MINOTAUR_SEM_FIFO_H_
/**
 * @file sem_fifo.h
 * @author Wolfhead
 */
#include <mutex>
#include <atomic>
#include "../common/sema.h"
#include "ring_buffer.h"

namespace ade { namespace queue {

template<typename T>
class SemFifo {
 public:
  SemFifo(uint64_t size)
      : ring_(size) 
      , producer_(0)
      , consumer_(0)
      , sema_read_(size, 0)
      , sema_write_(size, size) {
  }

  bool Push(const T& data) {
    if (!sema_write_.trywait()) return false;
    {
      std::lock_guard<std::mutex> lock(write_lock_);
      ring_.At(producer_) = data;
      ++producer_;
    }
    sema_read_.post();
    return true;
  }

  bool PushPri(const T& data) {
    if (!sema_write_.trywait()) return false;
    {
      std::lock_guard<std::mutex> lock(read_lock_);
      ring_.At(--consumer_) = data;
    }
    sema_read_.post();
    return true;
  }

  bool Pop(T* data, int32_t milliseconds = 0) {
    if (!sema_read_.wait(milliseconds)) return false;
    std::unique_lock<std::mutex> lock(read_lock_);
    *data = ring_.At(consumer_);
    ++consumer_;
    sema_write_.post();
    return true;
  }
  
  uint64_t Size() const {
    return RingBuffer<T>::GetDistance(ring_.Size(), consumer_, producer_);
  }

  uint64_t Capacity() const {
    return ring_.Size();
  }
  
  bool WouldBlock() const {
    return true;
  }

 private:
  RingBuffer<T> ring_;
  uint64_t producer_;
  uint64_t consumer_;

  std::mutex read_lock_;
  std::mutex write_lock_;

  SemaType sema_read_;
  SemaType sema_write_;
};

} //namespace queue
} //namespace ade

#endif // _MINOTAUR_SEM_FIFO_H_
