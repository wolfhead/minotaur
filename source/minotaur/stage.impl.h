/**
 * @file stage.cpp
 * @author Wolfhead
 */
#include "stage.h"
#include <sstream>

namespace ade {

template<typename HandlerType>
Stage<HandlerType>::Stage(
    HandlerType* prototype,
    uint32_t worker_count, 
    uint32_t queue_size,
    bool is_shared_queue,
    bool is_shared_handler)
    : prototype_(prototype)
    , worker_count_(worker_count)
    , queue_size_(queue_size)
    , is_shared_queue_(is_shared_queue)
    , is_shared_handler_(is_shared_handler)
    , shared_queue_(NULL) {
}

template<typename HandlerType>
Stage<HandlerType>::~Stage() {
  Stop();
  Wait();
  Destroy();

  if (prototype_) {
    delete prototype_;
  }
}

template<typename HandlerType>
void Stage<HandlerType>::Dump(std::ostream& os) const {
  os << "{\"worker_count\": " << worker_count_
     << ", \"queue_size\": " << queue_size_
     << ", \"is_shared_queue\": " << is_shared_queue_
     << ", \"is_shared_handler\": " << is_shared_handler_
     << ", \"stage_data\": [";

  for (StageDataType* data : data_) {
    os << "{\"queue_size\": " << data->queue->Size() << "}";
  }
  os << "}";
}

template<typename HandlerType>
std::string Stage<HandlerType>::ToString() const {
  std::ostringstream oss;
  Dump(oss);
  return oss.str();
}

template<typename HandlerType>
int Stage<HandlerType>::Start() {
  if (worker_count_ == 0) {
    return -1;
  }

  if (is_shared_queue_) {
    shared_queue_ = new MessageQueueType(queue_size_);
  }

  for (uint16_t i = 0; i != worker_count_; ++i) {
    HandlerType* handler = is_shared_handler_ ? prototype_ : prototype_->Clone();
    if (!is_shared_handler_) {
      uint16_t handler_id = i;
      handler->SetHandlerId(handler_id);
    } else {
      handler->SetHandlerId(0);
    }

    StageDataType* stage_data = new StageDataType;
    stage_data->queue = is_shared_queue_ ? shared_queue_ :  new MessageQueueType(queue_size_);
    stage_data->handler = handler;
    stage_data->running = true;
    stage_data->handler_id = handler->GetHandlerId();
    stage_data->thread = 
      new boost::thread(boost::bind(
            &HandlerType::Run, handler, stage_data));

    data_.push_back(stage_data);
  }
  return 0;
}

template<typename HandlerType>
int Stage<HandlerType>::Wait() {
  for (uint16_t i = 0; i != data_.size(); ++i) {
    StageDataType* data = data_[i];
    data->thread->join();
  }

  return 0;
}

template<typename HandlerType>
int Stage<HandlerType>::Stop() {
  for (size_t i = 0; i != data_.size(); ++i) {
    StageDataType* data = data_[i];
    data->running = false;
  }
  return 0;
}

template<typename HandlerType>
bool Stage<HandlerType>::Send(const MessageType& message) {
  MessageQueueType* queue = 
    data_.at(prototype_->Hash(message) % worker_count_)->queue;
  if (!queue->Push(message)) {
    return false;
  }
  return true;
}

template<typename HandlerType>
bool Stage<HandlerType>::SendPriority(const MessageType& message) {
  MessageQueueType* queue = 
    data_.at(prototype_->Hash(message) % worker_count_)->queue;
  if (!queue->PushPri(message)) {
    return false;
  }
  return true;
}

template<typename HandlerType>
int Stage<HandlerType>::Destroy() {
  for (size_t i = 0; i != data_.size(); ++i) {
    StageDataType* data = data_[i];
    if (!is_shared_queue_) {
      delete data->queue;
    }
    if (!is_shared_handler_) {
      delete data->handler;
    }
    delete data->thread;
    delete data;
  }
  data_.clear();

  if (shared_queue_) {
    delete shared_queue_;
    shared_queue_ = NULL;
  }

  return 0;
}

} //namespace ade
