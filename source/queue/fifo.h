#ifndef _MINOTAUR_FIFO_H_
#define _MINOTAUR_FIFO_H_
/**
 * @file fifo.h
 * @author Wolfhead
 */
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <chrono>
#include "ring_buffer.h"

namespace minotaur { namespace queue {

template<typename T>
class Fifo {
 public:
  Fifo(uint64_t size)
      : ring_(size) 
      , producer_(0)
      , consumer_(0)
      , count_(0) {
  }

  bool Push(const T& data) {
    if (Full()) return false;
    {
      std::lock_guard<std::mutex> lock(write_lock_);
      if (Full()) return false;
      ring_.At(producer_) = data;
      ++producer_;
      ++count_;
    }
    read_cond_.notify_one();
    return true;
  }

  bool Pop(T* data, int32_t milliseconds = 0) {
    std::unique_lock<std::mutex> lock(read_lock_);
    if (Empty()) {
      if (milliseconds == -1) {
        return false;
      }

      if (milliseconds) {
        read_cond_.wait_for(lock, std::chrono::milliseconds(milliseconds));
      } else {
        read_cond_.wait(lock);
      }
      if (Empty()) {
        return false;
      }
    }
    *data = ring_.At(consumer_);
    ++consumer_;
    --count_; 
    return true;
  }
  
  uint64_t Size() const {
    return RingBuffer<T>::GetDistance(consumer_, producer_);
  }

  uint64_t Capacity() const {
    return ring_.Size();
  }
  
  bool WouldBlock() const {
    return true;
  }

 private:
  bool Full() {return count_ == Capacity();}
  bool Empty() {return count_ == 0;}

  RingBuffer<T> ring_;
  uint64_t producer_;
  uint64_t consumer_;
  std::atomic<uint64_t> count_;

  std::mutex read_lock_;
  std::mutex write_lock_;

  std::condition_variable read_cond_;
};

} //namespace queue
} //namespace minotaur

#endif //_MINOTAUR_FIFO_H_
