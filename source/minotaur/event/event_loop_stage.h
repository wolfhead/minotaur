#ifndef _MINOTAUR_LOOP_STAGE_H_
#define _MINOTAUR_LOOP_STAGE_H_
/**
 * @file event_loop_stage.h
 * @author Wolfhead
 */
#include "../common/logger.h"
#include <boost/noncopyable.hpp>
#include "event_loop_thread.h"

namespace ade { namespace event {

class EventLoopStage : public boost::noncopyable {
 public:
  EventLoopStage(
      uint32_t thread_count, 
      uint32_t fd_size);

  ~EventLoopStage();

  int Start();
  int Stop();

  inline bool IsRunning() const {
    bool ret = false;
    for (const auto thread : event_loop_thread_) {
      ret |= thread->IsRunning();
    }
    return ret;
  }

  inline int RegisterRead(int fd, FdEventProc* proc, void* data) {
    return GetNotifier(fd, data).RegisterRead(fd, proc, data);
  }

  inline int RegisterWrite(int fd, FdEventProc* proc, void* data) {
    return GetNotifier(fd, data).RegisterWrite(fd, proc, data);
  }

  inline int RegisterReadWrite(int fd, FdEventProc* proc, void* data) {
    return GetNotifier(fd, data).RegisterReadWrite(fd, proc, data);
  }

  inline int RegisterClose(int fd, void* data) {
    return GetNotifier(fd, data).RegisterClose(fd, data); 
  }

  inline int UnregisterRead(int fd, void* data) {
    return GetNotifier(fd, data).UnregisterRead(fd);
  }

  inline int UnregisterWrite(int fd, void* data) {
    return GetNotifier(fd, data).UnregisterWrite(fd);
  }

  EventLoopNotifier& GetNotifier(int fd, void* data);

 private:
  LOGGER_CLASS_DECL(logger);

  int CreateEventLoopThread();
  int DestoryEventLoopThread();

  uint32_t thread_count_;
  uint32_t fd_size_;
  std::vector<EventLoopThread*> event_loop_thread_; 
};

} //namespace event
} //namespace ade

#endif // _MINOTAUR_LOOP_STAGE_H_
