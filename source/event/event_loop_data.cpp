/**
  @file event_loop_data.cpp
  @author Wolfhead
*/
#include "event_loop_data.h"
#include <stdlib.h>

namespace minotaur {
namespace event {

EventLoopData::EventLoopData(int set_size_) {
  stop = false;
  set_size = set_size_;
  file_events = new FileEvent[set_size];
  fired_events = new FiredEvent[set_size];

  for (int i = 0; i != set_size; ++i) {
    file_events[i].mask = EventType::EV_NONE;
  }
}

EventLoopData::~EventLoopData() {
  if (file_events) {
    delete [] file_events;
    file_events = NULL;
  }
}

} //namespace event
} //namespace mminotaur
