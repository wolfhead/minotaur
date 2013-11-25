#ifndef _MINOTAUR_EVENT_LOOP_DATA_H_
#define _MINOTAUR_EVENT_LOOP_DATA_H_
/**
  @file event_loop_data.h
  @author Wolfhead
*/

namespace minotaur {
namespace event {

class EventLoop;

struct EventType {
  enum {
    EV_NONE = 0,
    EV_READ = 1,
    EV_WRITE = 2
  };
};

typedef void FileEventProc(EventLoop *eventLoop, int fd, void *clientData, int mask);

struct FileEvent {
  int mask;
  FileEventProc* proc;
  void* client_data;
};

struct FiredEvent {
  int fd;
  int mask;
};

struct EventLoopData {
  EventLoopData(int set_size);
  ~EventLoopData();

  bool  stop;
  int   max_fd;
  int   set_size;
  FileEvent* file_events;
  FiredEvent* fired_events;
  void* impl_data;
};

} //namespace event
} //namespace minotaur

#endif // _MINOTAUR_EVENT_LOOP_DATA_H_
