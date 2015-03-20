/**
 * @file async_sequence_keeper.cpp
 * @author Wolfhead
 */
#include "async_sequence_keeper.h"
#include "io_message.h"

namespace minotaur {

LOGGER_CLASS_IMPL(logger, AsyncSequenceKeeper);

AsyncSequenceKeeper::AsyncSequenceKeeper(uint32_t timeout_in_msec) 
    : current_sequence_(0) 
    , timeout_sec_(timeout_in_msec / 1000)
    , timeout_usec_(timeout_in_msec % 1000) {
}

AsyncSequenceKeeper::~AsyncSequenceKeeper() {
}

int AsyncSequenceKeeper::Register(ProtocolMessage* message) {
  if (!registery_.insert(std::make_pair(message->sequence_id, message)).second) {
    MI_LOG_WARN(logger, "AsyncSequenceKeeper::Register duplicate sequence_id:"
        << message->sequence_id);
    return -1;
  }

  queue_.push_back(message);
  return 0;
}

ProtocolMessage* AsyncSequenceKeeper::Fetch(uint32_t sequence_id) {
  auto it = registery_.find(sequence_id);
  if (it == registery_.end()) {
    return NULL;
  }

  registery_.erase(it);
  queue_.erase(it->second);
  return it->second;
}

AsyncSequenceKeeper::QueueType AsyncSequenceKeeper::Timeout() {
  uint64_t current_timestamp = GetTimestamp(0, 0);
  QueueType result;
  ProtocolMessage* message = queue_.front();
  while (message && message->timestamp_ < current_timestamp) {
    result.push_back(message);
    queue_.pop_front();
    registery_.erase(message->sequence_id);
    message = queue_.front();
  }
  return result;
}

AsyncSequenceKeeper::QueueType AsyncSequenceKeeper::Clear() {
  QueueType result = queue_;
  registery_.clear();
  return result;
}

} //namespace minotaur
