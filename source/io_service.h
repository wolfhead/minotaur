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
class IOHandler;
class ServiceHandler;

template<typename T>
class Stage;

struct IOServiceConfig {
  IOServiceConfig() 
      : fd_count(65535) 
      , event_loop_worker(1)
      , io_worker(1)
      , io_queue_size(1024 * 128) 
      , service_worker(1)
      , service_queue_size(1024 * 128)
      , service_handler_prototype(NULL)
      , stack_size(2048) { 
  }

  uint32_t fd_count;
  uint8_t event_loop_worker;

  uint8_t io_worker;
  uint32_t io_queue_size;

  uint8_t service_worker;
  uint32_t service_queue_size;

  ServiceHandler* service_handler_prototype;

  uint32_t stack_size;
};

class IOService {
 public:
  typedef Stage<IOHandler> IOStage;
  typedef Stage<ServiceHandler> ServiceStage;

  IOService();
  ~IOService();

  int Init(const IOServiceConfig& config);
  int Start();
  int Stop();
  int Run();
  int CleanUp();

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
