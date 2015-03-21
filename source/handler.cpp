/**
 * @file handler.cpp
 * @author Wolfhead
 */
#include "handler.h"

namespace minotaur {

void HandlerSkeleton::Run(StageData* data) {
  uint32_t idle;
 
  OnStart();
  EventMessage message;
  while (data->running) {
    OnLoop();
 
    if (!data->pri_queue->Pop(&message)) {
      if (!data->queue->Pop(&message, 5)) {
        ++idle;
        OnIdle();
        continue;
      }
    }
 
    idle = 0;
    Handle(message);
  } 

  OnStop();
}

void HandlerSkeleton::OnStart() {
}

void HandlerSkeleton::OnLoop() {
}

void HandlerSkeleton::OnIdle() {
}

void HandlerSkeleton::OnStop() {
}

void HandlerSkeleton::Handle(const EventMessage& message) {
}

} //namespace minotaur
