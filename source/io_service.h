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
class IODescriptorFactory;

template<typename T>
class Stage;

struct IOServiceConfig {
  uint32_t fd_count;
  uint8_t event_loop_worker_;

  uint8_t io_worker_;
  uint32_t io_queue_size_;
};

class IOService {
 public:
  typedef Stage<IOHandlerFactory> IOStage;

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

  IODescriptorFactory* GetIODescriptorFactory() {
    return io_descriptor_factory_;
  }

 private:
  LOGGER_CLASS_DECL(logger);

  IOServiceConfig io_service_config_;
  event::EventLoopStage* event_loop_stage_;
  IOStage* io_stage_;
  IODescriptorFactory* io_descriptor_factory_;
};

} //namespace minotaur

#endif // _MINOTAUR_IO_SERVICE_H_
