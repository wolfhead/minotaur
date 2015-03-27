#ifndef _MINOTAUR_MESSAGE_QUEUE_H_
#define _MINOTAUR_MESSAGE_QUEUE_H_
/**
 * @file message_queue.h
 * @author Wolfhead
 */
#include <stdint.h>
#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>

namespace minotaur {

class spinlock {
private:
  typedef enum {Locked, Unlocked} LockState;
  boost::atomic<LockState> state_;

public:
  spinlock() : state_(Unlocked) {}

  void lock()
  {
    while (state_.exchange(Locked, boost::memory_order_acquire) == Locked) {
      /* busy-wait */
    }
  }
  void unlock()
  {
    state_.store(Unlocked, boost::memory_order_release);
  }
};


template <typename T>
class MessageQueue : public boost::noncopyable{
 public:
  MessageQueue(size_t capacity);

  ~MessageQueue();

  bool Produce(const T& value);

  bool Consume(T* value, uint32_t timeout_milliseconds);

 private:

  inline uint32_t GetNext(uint32_t current) {
    return current + 1 == capacity_ ? 0 : current + 1;
  }

  T* queue_;
  uint32_t capacity_;
  boost::atomic<uint32_t> producer_offset_;
  char padding_1[64];
  boost::atomic<uint32_t> consumer_offset_;
  char padding_2[64];

  boost::mutex consumer_lock_;
  boost::mutex producer_lock_;
  boost::condition_variable queue_condition_;

  spinlock spinlock_;
};

template<typename T>
MessageQueue<T>::MessageQueue(size_t capacity) 
    : queue_(new T[capacity])
    , capacity_(capacity)
    , producer_offset_(0)
    , consumer_offset_(0) {
}

template<typename T>
MessageQueue<T>::~MessageQueue() {
  delete [] queue_;
  queue_ = NULL;
}

template<typename T>
bool MessageQueue<T>::Produce(const T& value) {
  {
    boost::lock_guard<boost::mutex> lock(producer_lock_);
    uint32_t next_producer = GetNext(producer_offset_);
    if (next_producer == consumer_offset_.load(boost::memory_order_acquire)) {
      return false;
    }
    queue_[producer_offset_] = value;
    producer_offset_.store(next_producer, boost::memory_order_release);
  }

  queue_condition_.notify_all();
  return true;
}

/*
template<typename T>
bool MessageQueue<T>::Produce(const T& value) {
  spinlock_.lock();

  uint32_t next_producer = GetNext(producer_offset_);
  if (next_producer == consumer_offset_.load(boost::memory_order_acquire)) {
    spinlock_.unlock();
    return false;
  }
  queue_[producer_offset_] = value;
  producer_offset_.store(next_producer, boost::memory_order_release);
  spinlock_.unlock();

  queue_condition_.notify_all();
  return true;
}
*/


template<typename T>
bool MessageQueue<T>::Consume(T* value, uint32_t timeout_milliseconds) {
  boost::unique_lock<boost::mutex> lock(consumer_lock_);
  uint32_t next_consumer = GetNext(consumer_offset_);
  if (consumer_offset_ == producer_offset_.load()) {
    boost::system_time timeout = 
      boost::get_system_time() + boost::posix_time::milliseconds(timeout_milliseconds);
    if (false == queue_condition_.timed_wait(lock, timeout)) {
      return false;
    }
  }

  if (consumer_offset_ == producer_offset_.load()) {
    return false;
  }

  *value = queue_[consumer_offset_];
  consumer_offset_.store(next_consumer, boost::memory_order_release);

  return true;
}

/*
template<typename T>
bool MessageQueue<T>::Consume(T* value, uint32_t timeout_milliseconds) {
  uint32_t consumer_offset;
  uint32_t producer_offset;
  uint32_t next_consumer;
  uint32_t consumer_togo;

  do {
    consumer_offset = consumer_offset_;
    producer_offset = producer_offset_;
    if (consumer_offset == producer_offset) {
      boost::unique_lock<boost::mutex> lock(consumer_lock_);
      if (consumer_offset_ == producer_offset_) {
        boost::system_time timeout = 
          boost::get_system_time() + 
          boost::posix_time::milliseconds(timeout_milliseconds);  
        if (false == queue_condition_.timed_wait(lock, timeout)) {
          return false;
        } else {
          if (consumer_offset_ == producer_offset_) {
            return false;
          } else {
            consumer_offset = consumer_offset_;
            producer_offset = producer_offset_;
          }
        }
      } else {
        consumer_offset = consumer_offset_;
        producer_offset = producer_offset_;
      }
    }

    consumer_togo = consumer_offset;
    next_consumer = GetNext(consumer_offset);
  } while (
      !consumer_offset_.compare_exchange_weak(
        consumer_offset, 
        next_consumer,
        boost::memory_order_release));

  *value = queue_[consumer_togo];
  return true;
}
*/

} //namespacex minotaur

#endif //_MINOTAUR_MESSAGE_QUEUE_H_
