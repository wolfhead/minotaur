#ifndef _MINOTAUR_IO_SERVICE_H_
#define _MINOTAUR_IO_SERVICE_H_
/**
 * @file io_service.h
 * @author Wolfhead
 */
#include "event/event_loop_stage.h"
#include "common/fixed_size_pool.h"

namespace minotaur {

namespace event {
class EventLoopStage;
} //namespace event

class Channel;

class IOService {
 public:
  typedef FixedSizePool<Channel> ChannelPool;

  IOService();
  ~IOService();

  //currently hack this
  void SetEventLoopStage(event::EventLoopStage* stage) {
    event_loop_stage_ = stage;
  }

  event::EventLoopStage* GetEventLoopStage() const {
    return event_loop_stage_;
  }

  Channel* CreateChannel(int fd);
  Channel* GetChannel(uint64_t channel_id);
  bool DestoryChannel(uint64_t channel_id);

 private:
  event::EventLoopStage* event_loop_stage_;
  ChannelPool* channel_pool_;
};

} //namespace minotaur

#endif // _MINOTAUR_IO_SERVICE_H_
