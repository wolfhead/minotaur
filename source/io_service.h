#ifndef _MINOTAUR_IO_SERVICE_H_
#define _MINOTAUR_IO_SERVICE_H_
/**
 * @file io_service.h
 * @author Wolfhead
 */
#include "event/event_loop_stage.h"

namespace minotaur {

class IOService {
 public:
  IOService() {
  };

  ~IOService() {
  };

  //currently hack this
  void SetEventLoopStage(event::EventLoopStage* stage) {
    event_loop_stage_ = stage;
  }

  event::EventLoopStage* GetEventLoopStage() const {
    return event_loop_stage_;
  }

 private:
  event::EventLoopStage* event_loop_stage_;
};

} //namespace minotaur

#endif // _MINOTAUR_IO_SERVICE_H_
