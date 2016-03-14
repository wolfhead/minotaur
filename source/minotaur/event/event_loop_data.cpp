/**
  @file event_loop_data.cpp
  @author Wolfhead
*/
#include "event_loop_data.h"
#include <stdlib.h>
#include <string.h>

namespace ade {
namespace event {

EventLoopData::EventLoopData() 
    : init(false)
    , fd_size(0)
    , fd_events(NULL)
    , fired_events(NULL) {
}

EventLoopData::~EventLoopData() {
  if (fd_events) {
    delete [] fd_events;
    fd_events = NULL;
  }
  if (fired_events) {
    delete [] fired_events;
    fired_events = NULL;
  }
}

void EventLoopData::Init(uint32_t _fd_size) {
  fd_size = _fd_size;
  fd_events = new FdEvent[fd_size];
  fired_events = new FiredEvent[fd_size];

  memset(fd_events, 0, sizeof(FdEvent) * fd_size);
  memset(fired_events, 0, sizeof(FiredEvent) * fd_size);

  for (uint32_t i = 0; i != fd_size; ++i) {
    fd_events[i].mask = EventType::EV_NONE;
  }
  init = true;
}

void EventLoopData::Destroy() {
  if (fd_events) {
    delete [] fd_events;
    fd_events = NULL;
  }
  if (fired_events) {
    delete [] fired_events;
    fired_events = NULL;
  }  
}

} //namespace event
} //namespace made
