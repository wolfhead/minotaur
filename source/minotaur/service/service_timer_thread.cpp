/**
 * @file service_timer_thread.cpp
 * @author Wolfhead
 */
#include "service_timer_thread.h" 
#include <sys/prctl.h>
#include "../io_service.h"
#include "../net/io_message.h"
#include "service_handler.h"

namespace ade {

LOGGER_CLASS_IMPL(logger, ServiceTimerThread);

ServiceTimerThread::ServiceTimerThread(
    IOService* io_service,
    IOService::ServiceStage* service_stage) 
    : io_service_(io_service)
    , service_stage_(service_stage)
    , thread_(NULL)
    , running_(false) 
    , timer_(5, 1000 * 60 * 60) {
}

ServiceTimerThread::~ServiceTimerThread() {
  Stop();
}

int ServiceTimerThread::Start() {
  if (thread_) {
    MI_LOG_DEBUG(logger, "ServiceTimerThread::Start already running");
    return -1;
  }

  running_ = true;
  thread_ = new boost::thread(boost::bind(&ServiceTimerThread::Run, this));
  return 0;
}

int ServiceTimerThread::Stop() {
  if (!thread_) {
    return 0;
  }

  running_ = false;
  thread_->join();
  delete thread_;
  thread_ = NULL;
  return 0;
}

int ServiceTimerThread::ScheduleTask(const TimerFunc& func) {
  TimerMessage* message = MessageFactory::Allocate<TimerMessage>(func);
  if (!service_stage_->Send(message)) {
    LOG_ERROR(logger, "ServiceTimerThread::ScheduleTask Send fail");
    MessageFactory::Destroy(message);
    return -1;
  }
  return 0;
}

void ServiceTimerThread::Run() {
  LOG_INFO(logger, "ServiceTimerThread::Run Start");
  prctl(PR_SET_NAME, "service_timer");

  while (running_) {
    ProcessTimer();
    usleep(3000);
  }

  LOG_INFO(logger, "ServiceTimerThread::Run End");
}

void ServiceTimerThread::ProcessTimer() {
  lock_.lock();
  Timer::NodeType* timer_head = timer_.ProcessTimer();
  lock_.unlock();

  Timer::NodeType* current = timer_head;
  while (current) {
    if (current->active) {
      TimerMessage* message = MessageFactory::Allocate<TimerMessage>(std::move(current->data));
      if (!service_stage_->Send(message)) {
        LOG_ERROR(logger, "ServiceTimerThread::ProcessTimer Send fail");
        MessageFactory::Destroy(message);
      }
    }
    current = current->next;
  }
  if (timer_head) {
    timer_.DestroyTimerNode(timer_head);
  }

}

} //namespace ade
