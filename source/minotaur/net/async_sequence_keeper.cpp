/**
 * @file async_sequence_keeper.cpp
 * @author Wolfhead
 */
#include "async_sequence_keeper.h"
#include "../common/time_util.h"
#include "io_message.h"
#include "../matrix/matrix_scope.h"

namespace ade {

LOGGER_CLASS_IMPL(logger, AsyncSequenceKeeper);

AsyncSequenceKeeper::AsyncSequenceKeeper(uint32_t timeout_in_msec) 
    : current_sequence_(0) 
    , timeout_msec_(timeout_in_msec) {
}

AsyncSequenceKeeper::~AsyncSequenceKeeper() {
}

int AsyncSequenceKeeper::Register(ProtocolMessage* message) {
  message->time.timeout = Time::GetMillisecond() + timeout_msec_;

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

  ProtocolMessage* message = it->second;
  registery_.erase(it);

  queue_.erase(message);
  return message;
}

AsyncSequenceKeeper::QueueType AsyncSequenceKeeper::Timeout() {
  uint64_t current_timestamp = Time::GetMillisecond();

  QueueType result;
  ProtocolMessage* message = queue_.front();
  while (message && message->time.timeout < current_timestamp) {
    queue_.pop_front();

    result.push_back(message);
    registery_.erase(message->sequence_id);
    message = queue_.front();
  }
  return result;
}

AsyncSequenceKeeper::QueueType AsyncSequenceKeeper::Clear() {
  QueueType result = queue_;
  registery_.clear();
  queue_.clear();
  return result;
}

} //namespace ade
