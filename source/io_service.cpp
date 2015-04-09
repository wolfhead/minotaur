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
#include "net/socket_op.h"
#include "service/service_handler.h"
#include "coroutine/coro_all.h"

namespace minotaur {

LOGGER_CLASS_IMPL_NAME(logger, IOService, "IOService");
volatile IOService* IOService::current_ = NULL;

void IOServiceConfig::Dump(std::ostream& os) const {
  os << "{\"type\": \"IOServiceConfig\""
     << ", \"fd_count\": " << fd_count
     << ", \"event_loop_worker\": " << event_loop_worker
     << ", \"io_worker\": " << io_worker
     << ", \"io_queue_size\": " << io_queue_size
     << ", \"service_worker\": " << service_worker
     << ", \"service_queue_size\": " << service_queue_size
     << ", \"stack_size\": " << stack_size
     << "}";
}

IOService::IOService() 
    : event_loop_stage_(NULL)
    , io_stage_(NULL)
    , service_stage_(NULL) { 
}

IOService::~IOService() {
  if (service_stage_) {
    delete service_stage_;
    service_stage_ = NULL;
  }

  if (io_stage_) {
    delete io_stage_;
    io_stage_ = NULL;
  }

  if (event_loop_stage_) {
    delete event_loop_stage_;
    event_loop_stage_ = NULL;
  }

  current_ = NULL;
}

int IOService::Init(const IOServiceConfig& config) {

  SocketOperation::IgnoreSigPipe();

  ThreadLocalCorotineFactory::GlobalInit(config.stack_size); 

  io_service_config_ = config;
  event_loop_stage_ = 
    new event::EventLoopStage(
          config.event_loop_worker, 
          config.fd_count);
  io_stage_ = 
    new IOStage(
          new IOHandler(this), 
          config.io_worker, 
          config.io_queue_size);
  service_stage_ = 
    new ServiceStage(
          config.service_handler_prototype,
          config.service_worker,
          config.service_queue_size);
  current_ = this;
  return 0;
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
  MI_LOG_INFO(logger, "IOService::Run");

  service_stage_->Wait();
  return 0;
}

int IOService::CleanUp() {
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

std::ostream& operator << (std::ostream& os, const IOServiceConfig& config) {
  config.Dump(os);
  return os;
}

} //namespace minotaur
