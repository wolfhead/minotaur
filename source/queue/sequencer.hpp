#ifndef _MINOTAUR_QUEUE_SEQUENCER_HPP_
#define _MINOTAUR_QUEUE_SEQUENCER_HPP_
/**
  @file sequencer.hpp
  @author Wolfhead
*/

#include <boost/thread.hpp>
#include <mutex>

namespace minotaur { namespace queue {

#define LF_CAS(a_ptr, a_oldVal, a_newVal) \
__sync_bool_compare_and_swap(a_ptr, a_oldVal, a_newVal)

class PlainCursor;
class VolatileCursor;
class CASCursor;
class NoWaitStrategy;
class BusyLoopStrategy;

template<int RetryLoops, int NotifyCount>
class ConditionVariableStrategy;

template<typename T> class RingBuffer;
template<
  typename T, 
  typename ProducerCursor, 
  typename ConsumerCursor, 
  typename WaitStrategy> 
class Sequencer;

template<
  typename T,
  typename ProducerCursor,
  typename ConsumerCursor,
  typename WaitStrategy
  >
class Sequencer {
 public:
  Sequencer(uint64_t size) 
      : ring_(size) 
      , producer_curser_(0) 
      , consumer_curser_(0) {

  }

  bool Push(const T& data) {
    uint64_t producer_seq;
    BufferItem* item;
    bool ret = false;

    do {
      producer_seq = producer_curser_.Get();
      item = &ring_.At(producer_seq + 1);
      if (item->flag.load(boost::memory_order_acquire) != BufferItem::kEmptyItem) {
        if (producer_seq + 1 == consumer_curser_.Get() && 
            item->flag.load(boost::memory_order_acquire) != BufferItem::kEmptyItem) {
          return false;
        } else {
          continue;
        }
      }
      ret = producer_curser_.Set(producer_seq, producer_seq + 1);
    } while (!ret);

    item->value = data;
    item->flag.store(BufferItem::kOccupiedItem, boost::memory_order_release);

    wait_.Notify(consumer_curser_.Get(), producer_seq);

    return true;
  }

  bool Pop(T* data, uint32_t milliseconds = 0) {
    uint64_t consumer_seq = consumer_curser_.Get();
    uint16_t loop = 0;
    BufferItem* item;

    do {
      ++loop;
      consumer_seq = consumer_curser_.Get();
      item = &ring_.At(consumer_seq + 1);
      if (item->flag.load(boost::memory_order_acquire) != BufferItem::kOccupiedItem) {
        if (milliseconds == 0 
            ? wait_.Wait(loop) 
            : wait_.TimedWait(loop, milliseconds)) {
          continue;
        } else {
          return false;
        }
      }
      
      if (consumer_curser_.Set(consumer_seq, consumer_seq + 1)) {
         break;
      }
    } while (true);

    *data = item->value;
    item->flag.store(BufferItem::kEmptyItem, boost::memory_order_release);

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

  uint64_t Capacity() const {
    return ring_.Size();
  }

  bool WouldBlock() const {
    return wait_.WouldBlock();
  }

 private:
  struct BufferItem {
    enum {
      kEmptyItem = 0,
      kOccupiedItem = 1,
    };

    BufferItem() : flag(kEmptyItem) {};
    boost::atomic<char> flag;
    T value;
  };


  RingBuffer<BufferItem> ring_;

  ProducerCursor producer_curser_;
  ConsumerCursor consumer_curser_;
  WaitStrategy wait_;
};

template<typename T, typename WaitStrategy = NoWaitStrategy>
class SPSCQueue : public Sequencer<T, PlainCursor, PlainCursor, WaitStrategy> {
 public:
  typedef Sequencer<T, PlainCursor, PlainCursor, WaitStrategy> super;
  SPSCQueue(uint64_t size) : super(size) {};
};

template<typename T, typename WaitStrategy = NoWaitStrategy>
class SPMCQueue : public Sequencer<T, PlainCursor, CASCursor, WaitStrategy> {
 public:
  typedef Sequencer<T, PlainCursor, CASCursor, WaitStrategy> super;
  SPMCQueue(uint64_t size) : super(size) {};
};

template<typename T, typename WaitStrategy = NoWaitStrategy>
class MPSCQueue : public Sequencer<T, CASCursor, PlainCursor, WaitStrategy> {
 public:
  typedef Sequencer<T, CASCursor, PlainCursor, WaitStrategy> super;
  MPSCQueue(uint64_t size) : super(size) {};
};

template<typename T, typename WaitStrategy = NoWaitStrategy>
class MPMCQueue : public Sequencer<T, CASCursor, CASCursor, WaitStrategy> {
 public:
  typedef Sequencer<T, CASCursor, CASCursor, WaitStrategy> super;
  MPMCQueue(uint64_t size) : super(size) {};
};

class PlainCursor {
 public:
  PlainCursor() :sequence_(0) {
    padding_[0] = 0;//avoid warning
  }
  PlainCursor(uint64_t sequence) :sequence_(sequence) {}

  uint64_t Get() const {return sequence_;}
  bool Set(uint64_t /*original*/, uint64_t sequence) {sequence_ = sequence; return true;}
  void Inc() {++sequence_;}

 private:
  uint64_t sequence_;
  uint64_t padding_[8];
};

class VolatileCursor {
 public:
  VolatileCursor() :sequence_(0) {
    padding_[0] = 0;//avoid warning
  }
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
  uint64_t padding_[8];
};

class CASCursor {
 public:
  CASCursor() : sequence_(0) {
    padding_[0] = 0;//avoid warning
  }
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
  uint64_t padding_[8];
};

class NoWaitStrategy {
 public:
  void Notify(uint64_t consume, uint64_t produce) {}
  bool Wait(uint16_t loop) {return false;}
  bool TimedWait(uint16_t loop, uint32_t milliseconds) {return false;}
  bool WouldBlock() const {return false;}
};

class BusyLoopStrategy {
 public:
  void Notify(uint64_t consume, uint64_t produce) {}
  bool Wait(uint16_t loop) {return loop < 0xFFFF;}
  bool TimedWait(uint16_t loop, uint32_t milliseconds) {return loop < 0xFFFF;}
  bool WouldBlock() const {return true;}
};


template<int RetryLoops = 0, int NotifyCount = 0>
class ConditionVariableStrategy {
 public:
  void Notify(uint64_t consume, uint64_t produce) {
    if (NotifyCount && consume + NotifyCount < produce) return;
    cond_.notify_all();
  }
  bool Wait(uint16_t loop) {
    if (loop < RetryLoops) return true; 
    boost::unique_lock<boost::mutex> lock(lock_);
    cond_.wait(lock);
    return true;
  }

  bool TimedWait(uint16_t loop, uint32_t milliseconds) {
    if (loop < RetryLoops) return true; 
    boost::system_time timeout = 
          boost::get_system_time() + 
          boost::posix_time::milliseconds(milliseconds);  
    boost::unique_lock<boost::mutex> lock(lock_);
    return cond_.timed_wait(lock, timeout);
  }

  bool WouldBlock() const {return true;}

 private:
  boost::condition_variable cond_;
  boost::mutex lock_;
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
