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

template<typename T>
class FixedSizePool;

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
  typedef FixedSizePool<Channel> ChannelPool;
  typedef Stage<IOHandlerFactory> IOStage;

  IOService(const IOServiceConfig& config);
  ~IOService();

  int Start();
  int Stop();
  int Run();

  event::EventLoopStage* GetEventLoopStage() const {
    return event_loop_stage_;
  }

  IOStage* GetIOStage() const {
    return io_stage_;
  }

  Channel* CreateChannel(int fd);
  Channel* GetChannel(uint64_t channel_id);
  bool DestoryChannel(uint64_t channel_id);

 private:
  LOGGER_CLASS_DECL(logger);

  IOServiceConfig io_service_config_;
  event::EventLoopStage* event_loop_stage_;
  IOStage* io_stage_;
  ChannelPool* channel_pool_;
};

} //namespace minotaur

#endif // _MINOTAUR_IO_SERVICE_H_
