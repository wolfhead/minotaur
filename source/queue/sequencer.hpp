#ifndef _MINOTAUR_QUEUE_SEQUENCER_HPP_
#define _MINOTAUR_QUEUE_SEQUENCER_HPP_
/**
  @file sequencer.hpp
  @author Wolfhead
*/

namespace minotaur { namespace queue {

class PlainCursor {
 public:
  PlainCursor() :sequence_(0) {}

  uint64_t Get() const {return sequence_;}
  bool Set(uint64_t /*original*/, uint64_t sequence) {sequence_ = sequence; return true;}
  void Inc() {++sequence_;}

 private:
  uint64_t sequence_;
};

class VolatileCursor {
 public:
  VolatileCursor() :sequence_(0) {}

  uint64_t Get() const {return sequence_;}
  bool Set(uint64_t original, uint64_t sequence) {
    if (sequence_ == original) {
      sequence_ = sequence;
      return true;
    }

    return false;
  }
  void Inc() {++sequence_;}
 private:
  volatile uint64_t sequence_;
};

template<
  typename T
  >
class RingBuffer {
 public:
  RingBuffer(uint64_t size) 
      : size_(size)
      , mask_(size - 1)
      , buffer_(new T[size_]) {
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

 private:
  uint64_t size_;
  uint64_t mask_;
  T* buffer_;
};

template<
  typename T
  >
class Sequencer {
 public:
  Sequencer(uint64_t size) 
      : ring_(size) {

  }

  bool Push(const T& data) {
    uint64_t consumer_seq;
    uint64_t producer_seq;
    uint64_t claim_seq;

    do {
      consumer_seq = consumer_curser_.Get();
      producer_seq = producer_curser_.Get();
      claim_seq = producer_seq + 1;
      if (claim_seq == consumer_seq || claim_seq == consumer_seq + ring_.Size()) {
        return false;
      }
    } while (!producer_curser_.Set(producer_seq, claim_seq));

    ring_.At(claim_seq) = data;

    do {} while (!publisher_curser_.Set(producer_seq, claim_seq));

    return true;
  }


  bool Pop(T* data) {
    uint64_t consumer_seq = consumer_curser_.Get();
    uint64_t publisher_seq = publisher_curser_.Get();

    if (consumer_seq == publisher_seq) {
      return false;
    }

    *data = ring_.At(consumer_seq);

    consumer_curser_.Inc();
    return true;
  }

  uint64_t Size() const {
    uint64_t consumer_seq = consumer_curser_.Get();
    uint64_t publisher_seq = publisher_curser_.Get();

    if (publisher_seq >= consumer_seq) {
      return publisher_seq - consumer_seq;
    } else {
      return consumer_seq + ring_.Size() - publisher_seq + ring_.Size();
    }
  }

 private:

  RingBuffer<T> ring_;

  PlainCursor producer_curser_;
  PlainCursor publisher_curser_;
  PlainCursor consumer_curser_;
};






} //namespace queue
} //namespace minotaur

#endif // _MINOTAUR_QUEUE_SEQUENCER_HPP_
