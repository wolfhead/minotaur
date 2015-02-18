/**
 * @file io_service.cpp
 * @author Wolfhead
 */
#include "io_service.h"
#include "stage.h"
#include "event/event_loop_stage.h"
#include "net/channel.h"
#include "net/io_handler.h"
#include "net/io_descriptor_factory.h"

namespace minotaur {

LOGGER_CLASS_IMPL_NAME(logger, IOService, "IOService");

IOService::IOService(const IOServiceConfig& config) 
    : io_service_config_(config)
    , event_loop_stage_(
        new event::EventLoopStage(
          config.event_loop_worker_, 
          config.fd_count))
    , io_stage_(
        new IOStage(
          new IOHandlerFactory(this), 
          config.io_worker_, 
          config.io_queue_size_))
    , io_descriptor_factory_(
        new IODescriptorFactory(this)) {
}

IOService::~IOService() {
  delete io_descriptor_factory_;
  delete io_stage_;
  delete event_loop_stage_;
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

  return 0;
}

int IOService::Stop() {
  io_stage_->Stop();
  io_stage_->Wait();

  if (0 != event_loop_stage_->Stop()) {
    MI_LOG_ERROR(logger, "IOService::Stop event_loop fail");
  }

  return 0;
}


} //namespace minotaur
