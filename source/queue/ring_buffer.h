#ifndef _MINOTAUR_RING_BUFFER_H_
#define _MINOTAUR_RING_BUFFER_H_
/**
 * @file ring_buffer.h
 * @author Wolfhead
 */
#include <stdint.h>
#include <assert.h>

namespace minotaur { namespace queue {

template<typename T>
class RingBuffer {
 public:
  RingBuffer(uint64_t size) 
      : size_(size)
      , mask_(size - 1)
      , buffer_(new T[size_]) {
    assert((size_ & mask_) == 0);
  }

  ~RingBuffer() {
    if (buffer_) {
      delete [] buffer_;
      buffer_ = NULL;
    }
  }

  T& At(uint64_t index) {return buffer_[index & mask_];}
  const T& At(uint64_t index) const {return buffer_[index & mask_];}

  uint64_t Size() const {return size_;}
  uint64_t Index(uint64_t index) const {return index & mask_;}

  static uint64_t GetDistance(uint64_t begin, uint64_t end) {
    if (end >= begin) {
      return end - begin;
    } else {
      return 0xFFFFFFFFFFFFFFFF - (begin - end - 1);
    }  
  }

 private:
  uint64_t size_;
  uint64_t mask_;
  T* buffer_;
};

} //namespace queue
} //namespace minotaur

#endif //_MINOTAUR_RING_BUFFER_H_
