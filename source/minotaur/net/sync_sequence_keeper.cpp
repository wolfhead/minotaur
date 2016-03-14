/**
 * @file sync_sequence_keeper.cpp
 * @author Wolfhead
 */
#include "sync_sequence_keeper.h"
#include "../common/time_util.h"
#include "io_message.h"

namespace ade {

LOGGER_CLASS_IMPL(logger, SyncSequenceKeeper);

SyncSequenceKeeper::SyncSequenceKeeper(uint32_t timeout_in_msec) 
    : timeout_msec_(timeout_in_msec)
    , fired_(NULL) {
}

SyncSequenceKeeper::~SyncSequenceKeeper() {
}

int SyncSequenceKeeper::Register(ProtocolMessage* message) {
  message->time.timeout = Time::GetMillisecond() + timeout_msec_;
  queue_.push_back(message);
  return 0; 
}

ProtocolMessage* SyncSequenceKeeper::Fetch() {
  if (!fired_) {
    return NULL;
  } else {
    ProtocolMessage* fired = fired_;
    queue_.pop_front();
    fired_ = NULL;
    return fired;
  }
}

ProtocolMessage* SyncSequenceKeeper::Fired() {
  return fired_;
}

ProtocolMessage* SyncSequenceKeeper::Fire() {
  if (fired_) {
    return NULL;
  } else {
    return fired_ = queue_.front();
  }
}

SyncSequenceKeeper::QueueType SyncSequenceKeeper::Timeout() {
  if (!fired_) {
    return QueueType();
  }

  uint64_t current_timestamp = Time::GetMillisecond();
  if (fired_->time.timeout < current_timestamp) {
    return Clear();
  } else {
    return QueueType();
  }
}

SyncSequenceKeeper::QueueType SyncSequenceKeeper::Clear() {
  QueueType result(queue_);
  queue_.clear();
  fired_ = NULL;
  return result;
}

} //namespace ade
