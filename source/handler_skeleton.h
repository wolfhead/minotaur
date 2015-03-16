#ifndef _MINOTAUR_HANDLER_SKELETON_H_
#define _MINOTAUR_HANDLER_SKELETON_H_
/**
 * @file handler_skeleton.h
 * @author Wolfhead
 */
#include "stage.h"

namespace minotaur {


class HandlerSkeleton {
 public:
  int Start() {return 0;}

  int Stop() {return 0;}

  void OnLoopStart() {return;}

  void OnPerLoop() {return;}

  void OnIdle() {return;}
};

} //namespace minotaur

#endif // _MINOTAUR_HANDLER_SKELETON_H_
