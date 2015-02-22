#ifndef _MINOTAUR_IO_SERVICE_H_
#define _MINOTAUR_IO_SERVICE_H_
/**
 * @file io_service.h
 * @author Wolfhead
 */
#include <stdint.h>
#include "common/logger.h"

namespace minotaur {

namespace event {
class EventLoopStage;
} //namespace event

class Channel;
class IOHandlerFactory;
class ServiceHandlerFactory;

template<typename T>
class Stage;

struct IOServiceConfig {
  uint32_t fd_count;
  uint8_t event_loop_worker;

  uint8_t io_worker;
  uint32_t io_queue_size;

  uint8_t service_worker;
  uint32_t service_queue_size;

  ServiceHandlerFactory* service_handler_factory;
};

class IOService {
 public:
  typedef Stage<IOHandlerFactory> IOStage;
  typedef Stage<ServiceHandlerFactory> ServiceStage;

  IOService(const IOServiceConfig& config);
  ~IOService();

  int Start();
  int Stop();
  int Run();

  event::EventLoopStage* GetEventLoopStage() {
    return event_loop_stage_;
  }

  IOStage* GetIOStage() {
    return io_stage_;
  }

  ServiceStage* GetServiceStage() {
    return service_stage_;
  }

  void HandleSignal();

 private:
  LOGGER_CLASS_DECL(logger);

  IOServiceConfig io_service_config_;
  event::EventLoopStage* event_loop_stage_;
  IOStage* io_stage_;
  ServiceStage* service_stage_;

  static void StopCurrentIOService(int signal);
  static volatile IOService* current_;
};

} //namespace minotaur

#endif // _MINOTAUR_IO_SERVICE_H_
