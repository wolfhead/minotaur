#ifndef _MINOTAUR_EVENT_LOOP_NOTIFIER_H_
#define _MINOTAUR_EVENT_LOOP_NOTIFIER_H_
/**
 * @file event_loop_notifier.h
 * @author Wolfhead
 */
#include "event_loop_data.h"
#include "../common/logger.h"
#include "../queue/sequencer.hpp"


namespace minotaur { namespace event {

class EventLoopNotifier;
class EventLoop;

struct NotifyMessage {
  enum {
    ADD_READ = 1,
    ADD_WRITE = 2,
    ADD_READ_WRITE = 3,
    ADD_CLOSE = 4,
    REMOVE_READ = 5,
    REMOVE_WRITE = 6,
  };

  int32_t       fd;
  uint32_t      mask;
  void*         data;
  FdEventProc*  proc;

  void Dump(std::ostream& os) const;
};

class EventLoopNotifier {
 public:
  enum {
    kDescriptorFD = -2,
  };

  EventLoopNotifier(EventLoop* event_loop);
  ~EventLoopNotifier();

  int Init();

  int Notify(int fd, uint32_t mask, FdEventProc* proc, void* data);

  int RegisterRead(int fd, FdEventProc* proc, void* data);

  int RegisterWrite(int fd, FdEventProc* proc, void* data);

  int RegisterReadWrite(int fd, FdEventProc* proc, void* data);

  int RegisterClose(int fd);

  int UnregisterRead(int fd);

  int UnregisterWrite(int fd);

 private:
  typedef queue::MPSCQueue<
    NotifyMessage, 
    queue::NoWaitStrategy> MessageQueueType;

  LOGGER_CLASS_DECL(logger);

  static void EventLoopNotifierProc(
      EventLoop* event_loop,
      int fd,
      void* data,
      uint32_t mask);

  static int NotifyEventLoop(
      EventLoop* event_loop, 
      const NotifyMessage& message);

  EventLoop* event_loop_;

  int event_fd_;
  MessageQueueType queue_;
};

std::ostream& operator << (std::ostream& os, const NotifyMessage& notify_message);

} //namespace event
} //namespace minotaur

#endif //_MINOTAUR_EVENT_LOOP_NOTIFIER_H_
