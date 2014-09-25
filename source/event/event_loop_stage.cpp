/**
 * @file event_loop_stage.cpp
 * @author Wolfhead
 */
#include "event_loop_stage.h"

namespace minotaur { namespace event {

LOGGER_CLASS_IMPL_NAME(logger, EventLoopStage, "event.EventLoopStage");

EventLoopStage::EventLoopStage(uint32_t thread_count, uint32_t fd_size) 
    : thread_count_(thread_count)
    , fd_size_(fd_size) {
}

EventLoopStage::~EventLoopStage() {
  Stop();
}

int EventLoopStage::Start() {
  if (0 != CreateEventLoopThread()) {
    return -1;
  }

  for (auto thread : event_loop_thread_) {
    if (0 != thread->Start()) {
      return -1;
    }
  }

  return 0;
}

int EventLoopStage::Stop() {  
  return DestoryEventLoopThread();
}

int EventLoopStage::CreateEventLoopThread() {
  for (uint32_t i = 0; i != thread_count_; ++i) {
    event_loop_thread_.push_back(new EventLoopThread(fd_size_)); 
  }
  return 0;
}

int EventLoopStage::DestoryEventLoopThread() {
  for (uint32_t i = 0; i != event_loop_thread_.size(); ++i) {
    delete event_loop_thread_[i];
    event_loop_thread_[i] = NULL;
  }
  return 0;
}

} //namespace event
} //namespcae minotaur
