#ifndef _MINOTAUR_QUEUE_SEQUENCER_HPP_
#define _MINOTAUR_QUEUE_SEQUENCER_HPP_
/**
  @file sequencer.hpp
  @author Wolfhead
*/

#include <boost/thread.hpp>

namespace minotaur { namespace queue {

#define LF_CAS(a_ptr, a_oldVal, a_newVal) \
__sync_bool_compare_and_swap(a_ptr, a_oldVal, a_newVal)


class PlainCursor;
class VolatileCursor;
class CASCursor;
template<typename T> class RingBuffer;
template<typename T, typename ProducerCursor, typename ConsumerCursor> class Sequencer;

template<
  typename T,
  typename ProducerCursor,
  typename ConsumerCursor
  >
class Sequencer {
 public:
  Sequencer(uint64_t size) 
      : ring_(size) 
      , producer_curser_(0xFFFFFFFFFFFFFFF0) 
      , consumer_curser_(0xFFFFFFFFFFFFFFF0) {

  }

  bool Push(const T& data) {
    uint64_t producer_seq;
    BufferItem* item;

    do {
      producer_seq = producer_curser_.Get();
      item = &ring_.At(producer_seq + 1);
      if (item->flag != 0) {
        return false;
      }
    } while (!producer_curser_.Set(producer_seq, producer_seq + 1));

    item->value = data;
    item->flag.store(2, boost::memory_order_release);

    return true;
  }


  bool Pop(T* data) {

    uint64_t consumer_seq = consumer_curser_.Get();
    BufferItem* item;

    do {
      consumer_seq = consumer_curser_.Get();
      item = &ring_.At(consumer_seq + 1);
      if (item->flag != 2) {
        return false;
      }
    } while (!consumer_curser_.Set(consumer_seq, consumer_seq + 1));

    *data = item->value;
    item->flag.store(0, boost::memory_order_release);

    return true;
  }

  uint64_t Size() const {
    uint64_t consumer_index = consumer_curser_.Get();
    uint64_t producer_index = producer_curser_.Get();

    if (producer_index >= consumer_index) {
      return producer_index - consumer_index;
    } else {
      return 0xFFFFFFFFFFFFFFFF - (consumer_index - producer_index - 1);
    }
  }

 private:
  struct BufferItem {
    BufferItem() : flag(0) {};

    boost::atomic<char> flag;
    T value;
  };


  RingBuffer<BufferItem> ring_;

  ProducerCursor producer_curser_;
  ConsumerCursor consumer_curser_;
};

template<typename T>
class SPSCQueue : public Sequencer<T, PlainCursor, PlainCursor> {
 public:
  typedef Sequencer<T, PlainCursor, PlainCursor> super;
  SPSCQueue(uint64_t size) : super(size) {};
};

template<typename T>
class MPSCQueue : public Sequencer<T, CASCursor, PlainCursor> {
 public:
  typedef Sequencer<T, CASCursor, PlainCursor> super;
  MPSCQueue(uint64_t size) : super(size) {};
};

template<typename T>
class MPMCQueue : public Sequencer<T, CASCursor, CASCursor> {
 public:
  typedef Sequencer<T, CASCursor, CASCursor> super;
  MPMCQueue(uint64_t size) : super(size) {};
};

class PlainCursor {
 public:
  PlainCursor() :sequence_(0) {}
  PlainCursor(uint64_t sequence) :sequence_(sequence) {}

  uint64_t Get() const {return sequence_;}
  bool Set(uint64_t /*original*/, uint64_t sequence) {sequence_ = sequence; return true;}
  void Inc() {++sequence_;}

 private:
  uint64_t sequence_;
  uint64_t padding_[7];
};

class VolatileCursor {
 public:
  VolatileCursor() :sequence_(0) {}
  VolatileCursor(uint64_t sequence) :sequence_(sequence) {}

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
  uint64_t padding_[7];
};

class CASCursor {
 public:
  CASCursor() : sequence_(0) {}
  CASCursor(uint64_t sequence) :sequence_(sequence) {}

  uint64_t Get() const {return sequence_;}
  bool Set(uint64_t original, uint64_t sequence) {return LF_CAS(&sequence_, original, sequence);}
  void Inc() {
    uint64_t original;
    uint64_t inc;
    do {
      original = sequence_;
      inc = original + 1;
    } while (!Set(original, inc));
  }
 private:
  volatile uint64_t sequence_;
  uint64_t padding_[7];
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

} //namespace queue
} //namespace minotaur

#endif // _MINOTAUR_QUEUE_SEQUENCER_HPP_
