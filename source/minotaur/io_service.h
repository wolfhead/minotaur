#ifndef _MINOTAUR_IO_SERVICE_H_
#define _MINOTAUR_IO_SERVICE_H_
/**
 * @file io_service.h
 * @author Wolfhead
 */
#include <stdint.h>
#include "common/logger.h"

namespace ade {

namespace event {
class EventLoopStage;
} //namespace event

class Channel;
class IOHandler;
class ServiceHandler;
class ServiceTimerThread;

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
      , service_timer_worker(0)
      , service_timer_queue_size(0)
      , service_handler_prototype(NULL)
      , stack_size(2048) 
      , matrix_token_bucket(16)
      , matrix_queue_bucket(16)
      , matrix_queue_size(1024 * 512) { 
  }

  // event loop
  uint32_t fd_count;
  uint8_t event_loop_worker;

  // io stage
  uint8_t io_worker;
  uint32_t io_queue_size;

  // service stage
  uint8_t service_worker;
  uint32_t service_queue_size;

  // service timer stage
  uint8_t service_timer_worker;
  uint32_t service_timer_queue_size;
  ServiceHandler* service_handler_prototype;

  // coroutine 
  uint32_t stack_size;

  // matrix
  uint32_t matrix_token_bucket;
  uint32_t matrix_queue_bucket;
  uint32_t matrix_queue_size;

  void Dump(std::ostream& os) const;
};

class IOService {
 public:
  typedef Stage<IOHandler> IOStage;
  typedef Stage<ServiceHandler> ServiceStage;
  typedef Stage<ServiceHandler> ServiceTimerStage;

  IOService();
  ~IOService();

  int Init(const IOServiceConfig& config);
  int Start();
  int Stop();
  int Run();
  int CleanUp();

  inline event::EventLoopStage* GetEventLoopStage() {
    return event_loop_stage_;
  }

  inline IOStage* GetIOStage() {
    return io_stage_;
  }

  inline ServiceStage* GetServiceStage() {
    return service_stage_;
  }
  
  inline ServiceTimerStage* GetServiceTimerStage() {
    return service_timer_stage_;
  }

  inline ServiceTimerThread* GetServiceTimerThread() {
    return service_timer_thread_;
  }

  const std::string& GetMatrixReport();

  void HandleSignal();

 private:
  LOGGER_CLASS_DECL(logger);

  IOServiceConfig io_service_config_;
  event::EventLoopStage* event_loop_stage_;
  IOStage* io_stage_;
  ServiceStage* service_stage_;
  ServiceTimerStage* service_timer_stage_;
  ServiceTimerThread* service_timer_thread_;

  static void StopCurrentIOService(int signal);
  static volatile IOService* current_;
};

std::ostream& operator << (std::ostream& os, const IOServiceConfig& config);

} //namespace ade

#endif // _MINOTAUR_IO_SERVICE_H_
