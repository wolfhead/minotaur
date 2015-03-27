/**
 * @file stage.cpp
 * @author Wolfhead
 */
#include "stage.h"

namespace minotaur {

template<typename HandlerType>
Stage<HandlerType>::Stage(
    HandlerType* prototype,
    uint32_t worker_count, 
    uint32_t queue_size)
    : prototype_(prototype)
    , worker_count_(worker_count)
    , queue_size_(queue_size) {
}

template<typename HandlerType>
Stage<HandlerType>::~Stage() {
  if (prototype_) {
    delete prototype_;
  }

  Stop();
  Wait();
}

template<typename HandlerType>
int Stage<HandlerType>::Start() {
  for (uint16_t i = 0; i != worker_count_; ++i) {
    HandlerType* handler = prototype_->Clone();
    uint16_t handler_id = i + 1;

    StageDataType* stage_data = new StageDataType;
    stage_data->queue = new MessageQueueType(queue_size_);
    stage_data->pri_queue = new PriorityMessageQueueType(queue_size_);
    stage_data->handler = handler;
    stage_data->running = true;
    stage_data->handler_id = handler_id;
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

  Destroy();
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
  PriorityMessageQueueType* queue = 
    data_.at(prototype_->Hash(message) % worker_count_)->pri_queue;
  if (!queue->Push(message)) {
    return false;
  }
  return true;
}

template<typename HandlerType>
int Stage<HandlerType>::Destroy() {
  for (size_t i = 0; i != data_.size(); ++i) {
    StageDataType* data = data_[i];
    delete data->queue;
    delete data->pri_queue;
    delete data->thread;
    delete data->handler;
    delete data;
  }

  data_.clear();
  return 0;
}

} //namespace minotaur
