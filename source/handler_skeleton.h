#ifndef _MINOTAUR_HANDLER_SKELETON_H_
#define _MINOTAUR_HANDLER_SKELETON_H_
/**
 * @file handler_skeleton.h
 * @author Wolfhead
 */
#include "stage.h"

namespace minotaur {

template<typename T, bool ShareHandler = false, bool ShareMessageQueue = false>
class HandlerSkeleton {
 public:
  typedef HandlerSkeleton self;
  typedef minotaur::Stage<self> StageType;
  typedef T MessageType;

  static const bool share_handler = ShareHandler;
  static const bool share_queue = ShareMessageQueue;

  static uint32_t HashMessage(const MessageType& /*message*/, uint32_t /*worker_count*/) {
    return 0;
  }

  void SetStage(StageType* stage) {stage_ = stage;}

  void Handle(const MessageType& message) {
  }

 private:
  StageType* stage_;  
};

} //namespace minotaur

#endif // _MINOTAUR_HANDLER_SKELETON_H_
