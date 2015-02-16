#ifndef _MINOTAUR_EVENT_LOOP_THREAD_H_
#define _MINOTAUR_EVENT_LOOP_THREAD_H_
/**
 * @file event_loop_thread.h
 * @author Wolfhead
 */
#include <stdint.h>
#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>
#include "event_loop.h"
#include "event_loop_notifier.h"
#include "../common/logger.h"

namespace minotaur { namespace event {

class EventLoopThread : public boost::noncopyable {
 public:
  EventLoopThread(uint32_t fd_size);

  ~EventLoopThread();

  int Start();

  int Stop();

  inline bool IsRunning() const {
    return running_; 
  }

  inline int RegisterRead(int fd, FdEventProc* proc, void* data) {
    return GetNotifier().RegisterRead(fd, proc, data);
  }

  inline int RegisterWrite(int fd, FdEventProc* proc, void* data) {
    return GetNotifier().RegisterWrite(fd, proc, data);
  }

  inline int UnregisterRead(int fd) {
    return GetNotifier().UnregisterRead(fd);
  }

  inline int UnregisterWrite(int fd) {
    return GetNotifier().UnregisterWrite(fd);
  }

  inline EventLoopNotifier& GetNotifier() {
    return event_loop_notifier_;
  }

 private:

  LOGGER_CLASS_DECL(logger);

  void Run(); 

  volatile bool running_;
  uint32_t fd_size_;
  boost::thread* thread_;
  EventLoop event_loop_;
  EventLoopNotifier event_loop_notifier_;
};

} //namespace event
} //namespace minotaur

#endif // _MINOTAUR_EVENT_LOOP_THREAD_H_
