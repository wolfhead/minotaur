/**
 * @file io_service.cpp
 * @author Wolfhead
 */
#include "io_service.h"
#include <signal.h>
#include "stage.h"
#include "event/event_loop_stage.h"
#include "net/channel.h"
#include "net/io_handler.h"
#include "service/service_handler.h"

namespace minotaur {

LOGGER_CLASS_IMPL_NAME(logger, IOService, "IOService");
volatile IOService* IOService::current_ = NULL;

IOService::IOService(const IOServiceConfig& config) 
    : io_service_config_(config)
    , event_loop_stage_(
        new event::EventLoopStage(
          config.event_loop_worker, 
          config.fd_count))
    , io_stage_(
        new IOStage(
          new IOHandlerFactory(this), 
          config.io_worker, 
          config.io_queue_size)) 
    , service_stage_(
        new ServiceStage(
          config.service_handler_factory->BindIOService(this),
          config.service_worker,
          config.service_queue_size)) {
  current_ = this;
}

IOService::~IOService() {
  delete service_stage_;
  delete io_stage_;
  delete event_loop_stage_;
  current_ = NULL;
}

int IOService::Start() {
  if (0 != event_loop_stage_->Start()) {
    MI_LOG_ERROR(logger, "IOService::Start event_loop fail");
    return -1;
  }

  if (0 != io_stage_->Start()) {
    MI_LOG_ERROR(logger, "IOService::Start io_stage fail");
    return -1;
  }

  if (0 != service_stage_->Start()) {
    MI_LOG_ERROR(logger, "IOService::Start service_stage fail");
    return -1;
  }

  return 0;
}

int IOService::Run() {
  service_stage_->Wait();

  io_stage_->Stop();
  io_stage_->Wait();

  if (0 != event_loop_stage_->Stop()) {
    MI_LOG_ERROR(logger, "IOService::Stop event_loop fail");
  }
  return 0;
}

int IOService::Stop() {
  service_stage_->Stop();
  return 0;
}

void IOService::HandleSignal() {
  struct sigaction sa;
  sa.sa_handler = SIG_IGN;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGPIPE, &sa, 0);
  sigaction(SIGHUP, &sa, 0);
  sigaction(SIGCHLD, &sa, 0);
  signal(SIGINT, &IOService::StopCurrentIOService);
  signal(SIGTERM, &IOService::StopCurrentIOService);
}

void IOService::StopCurrentIOService(int signal) {
  MI_LOG_INFO(logger, "IOService::StopCurrentIOService signal:" << signal);

  if (current_) {
    IOService* current = (IOService*)current_;
    current->Stop();
  }
}

} //namespace minotaur
