#ifndef _MINOTAUR_MESSAGE_QUEUE_H_
#define _MINOTAUR_MESSAGE_QUEUE_H_
/**
 * @file message_queue.h
 * @author Wolfhead
 */
#include <stdint.h>
#include <boost/thread.hpp>

namespace minotaur {

template <typename T>
class MessageQueue {
 public:
  MessageQueue(size_t capacity) 
      : queue_(new T[capacity])
      , capacity_(capacity)
      , producer_offset_(0)
      , consumer_offset_(0) {
  }

  ~MessageQueue() {
    delete [] queue_;
    queue_ = NULL;
  }

  bool Produce(const T& value) {
    {
      boost::lock_guard<boost::mutex> lock(producer_lock_);
      uint32_t next_producer = GetNext(producer_offset_);
      if (next_producer == consumer_offset_.load()) {
        return false;
      }
      queue_[producer_offset_] = value;
      producer_offset_.store(next_producer, boost::memory_order_release);
    }
    //printf("%u %u\n", producer_offset_, consumer_offset_);

    NotifyConsumer();
    return true;
  }

  bool Consume(T* value, uint32_t timeout_milliseconds) {
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

 private:
  void NotifyConsumer() {
    queue_condition_.notify_all();
  }

  inline uint32_t GetNext(uint32_t current) {
    return current + 1 == capacity_ ? 0 : current + 1;
  }

  T* queue_;
  uint32_t capacity_;
  boost::atomic<uint32_t> producer_offset_;
  boost::atomic<uint32_t> consumer_offset_;

  boost::mutex consumer_lock_;
  boost::mutex producer_lock_;
  boost::condition_variable queue_condition_;
};

} //namespacex minotaur

#endif //_MINOTAUR_MESSAGE_QUEUE_H_
