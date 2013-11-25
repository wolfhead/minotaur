/**
  @file timer_loop.cpp
  @author Wolfhead
*/

namespace minotaur {
namespace event {

TimerWheel::TimerWheel() 
    : running(false)
    , slots_(NULL)
    , events_(NULL) {

}

TimerWheel::TimerWheel() {
  Stop();

  Destroy();
}

int TimerWheel::Init(int wheel_size; int interval, int set_size) {
  size_ = RoundUpSize(wheel_size);
  interval_ = interval;
  slots_ = new TimerSlot[size_];
  events_ = new StaticListPool<TimerEvent>(set_size);

  // set timer id
  TimerEvent* p;
  for (size_t i = 0; i != set_size; ++i) {
    if (events_->At(i, &p)) {
      p->id = i;
    } else {
      return -1;
    }
  }

  return 0;
}

int TimerWheel::Destroy() {
  if (slots_) {
    delete [] slots_;
    slots_ = NULL;
  }

  if (events_) {
    delete events_;
    events_ = NULL;
  }
}

int TimerWheel::Start() {

}

int TimerWheel::Stop() {
  
}

int TimerWheel::AddEvent(
    int64_t when_sec, 
    int64_t when_msec, 
    TimerProc* proc, 
    void* client_data) {

  TimerEvent* p = NULL;
  if (!events_->Alloc(&p)) {
    return -1;
  }

  p->when_sec = when_sec;
  p->when_msec = when_msec;
  p->proc = proc;
  p->client_data = client_data;


}


uint32_t TimerWheel::RoundUpSize(uint32_t size) const {
  uint32_t mask = 0x80000000;
  for (;mask != 0; mask = mask >> 1) {
    if (size & mask) {
      break;
    }
  }

  if (size & ~mask) {
    return mask << 1;
  } else {
    return mask;
  }
}

void TimerWheel::GetCurrentTime(int64_t* sec, int64_t* usec) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  *sec = tv.tv_sec;
  *usec = tv.tv_usec;
}

} //namespace event
} //namespace minotaur
