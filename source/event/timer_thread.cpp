/**
 * @file timer_thread.cpp
 * @author Wolfhead
 */
#include "timer_thread.h"
#include <sys/prctl.h>

namespace minotaur { namespace event {

LOGGER_CLASS_IMPL_NAME(logger, TimerThread, "event.TimerThread");

TimerThread::TimerThread(uint32_t per_tick, uint32_t total_ticks) 
    : running_(false)
    , timer_(per_tick, total_ticks) {
}

TimerThread::~TimerThread() {
  Stop();
}

int TimerThread::Start() {
  if (running_) {
    MI_LOG_WARN(logger, "TimerThread::Start already running");
    return -1;
  }

  running_ = true;
  thread_ = new boost::thread(boost::bind(&TimerThread::Run, this));
  return 0;
}

int TimerThread::Stop() {
  if (!thread_) {
    return 0;
  }

  running_ = false;
  thread_->join();
  delete thread_;
  thread_ = NULL;

  return 0;
}

void TimerThread::Run() {
  MI_LOG_INFO(logger, "TimerThread::Run Start");
  prctl(PR_SET_NAME, "timer_loop");

  while (running_) {
    TimerNodeType* node = timer_.ProcessTimer();
    ProcessTimer(node);
    timer_.DestroyTimerNode(node);
    usleep(1000);
  }

  MI_LOG_INFO(logger, "TimerThread::Run Stop");
}

void TimerThread::ProcessTimer(TimerNodeType* list_head) {
  while (list_head) {
    list_head->data.proc(list_head->active, list_head->data.client_data);
    list_head = list_head->next;
  }
}


} // nmamespace event
} // namespace minotaur
