#ifndef MINOTUAR_COMMON_SEMA_H_
#define MINOTAUR_COMMON_SEMA_H_

#include <stdlib.h>
#include <stdint.h>
#include <semaphore.h>

namespace ade {

namespace {
inline void TimespecConvert(int32_t milliseconds, struct timespec *ts) {
  struct timeval tv;
  if (0 == gettimeofday(&tv, NULL)) {
    ts->tv_sec = tv.tv_sec + milliseconds / 1000;
    ts->tv_nsec = (suseconds_t) (tv.tv_usec*1000 + milliseconds % 1000 * 1000000);
    if (ts->tv_nsec >= 1000000000) {
      ts->tv_sec++;
      ts->tv_nsec -= 1000000000;
    }
  } 
}
} //namespace

class SemaType {
 public:
  SemaType(uint32_t /*capacity*/, uint32_t count) {
    sem_init(&sema_, 0, count);
  }

  ~SemaType() {
    sem_destroy(&sema_);
  }

  bool post() {
    return (sem_post(&sema_) == 0);
  }

  bool wait(int32_t milliseconds) {
    if (milliseconds == 0) {
      return (sem_wait(&sema_) == 0);
    }
    if (milliseconds > 0) {
      struct timespec ts;
      TimespecConvert(milliseconds, &ts);
      return (sem_timedwait(&sema_, &ts) == 0);
    }
    return (sem_trywait(&sema_) == 0);
  }

  bool trywait() {
    return sem_trywait(&sema_) == 0;
  }

 private:
  sem_t sema_;

  SemaType(const SemaType&);
  SemaType& operator=(const SemaType&);
};

} //namespace ade

#endif//MINOTAUR_COMMON_SEMA_H_

