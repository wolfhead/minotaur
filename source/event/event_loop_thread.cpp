/**
 * @file evnet_loop_thread.cpp
 * @author Wolfhead
 */
#include "event_loop_thread.h"
#include <sys/prctl.h>
#include <boost/bind.hpp>
#include "../common/system_error.h"

namespace minotaur { namespace event { 

LOGGER_CLASS_IMPL_NAME(logger, EventLoopThread, "event.EventLoopThread");

EventLoopThread::EventLoopThread(
    uint32_t fd_size) 
    : running_(false)
    , fd_size_(fd_size)
    , thread_(NULL) 
    , event_loop_notifier_(&event_loop_) {
}

EventLoopThread::~EventLoopThread() {
  Stop();
}

int EventLoopThread::Start() {
  if (running_) {
    LOG_WARN(logger, "EventLoopThread::Start already running");
    return -1;
  }

  if (0 != event_loop_.Init(fd_size_)) {
    LOG_ERROR(logger, "EventLoopThread::Start event loop init fail");
    return -1;
  }

  if (0 != event_loop_notifier_.Init()) {
    LOG_ERROR(logger, "EventLoopThread::Start event_loop_notifier init fail");
    return -1;
  }

  running_ = true;
  thread_ = new boost::thread(boost::bind(&EventLoopThread::Run, this));

  return 0;
}

int EventLoopThread::Stop() {
  if (!thread_) {
    return 0;
  }

  running_ = false;
  thread_->join();
  delete thread_;
  thread_ = NULL;

  return 0;
}

void EventLoopThread::Run() {
  LOG_INFO(logger, "EventLoopThread::Run Start");
  prctl(PR_SET_NAME, "event");
  event_loop_notifier_.SetWorkingThreadId(EventLoopNotifier::GetThreadId());

  while (running_) {
    if (event_loop_.ProcessEvent(1000) < 0) {
      LOG_ERROR(logger, "EventLoopThread::Run ProcessEvent fail with:"
          << SystemError::FormatMessage());
    } 

    event_loop_notifier_.Process();
  }

  LOG_INFO(logger, "EventLoopThread::Run Stop");
}

} //namespace event
} //namespace monitaur
