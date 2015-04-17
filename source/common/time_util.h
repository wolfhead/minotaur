#ifndef _MINOTAUR_COMMON_TIME_UTIL_H_
#define _MINOTAUR_COMMON_TIME_UTIL_H_
/**
 * @file time_util.h
 * @author Wolfhead
 *
 * copy from acapella
 */
#include <time.h>
#include <sys/time.h>
#include <memory.h>
#include <string>

namespace minotaur {

class Time {
 public:
  typedef uint64_t Millisecond;
  typedef uint64_t Microsecond;
  typedef time_t TimeVal;

  inline static Microsecond GetMillisecond() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return  (tv.tv_usec/1000 + tv.tv_sec*1000);
  }

  inline static Microsecond GetMicrosecond() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_usec + tv.tv_sec * 1000000);
  }

  inline static TimeVal Now() {
    return time(NULL);
  }

  inline static TimeVal GetGreenwishSecondOffset() {
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    return tz.tz_minuteswest * 60;
  }

  template<size_t bufSize>
  static std::string Format(const char * format, TimeVal sec = Time::Now()) {
    char buffer[bufSize];
    tm tmpTm;
    localtime_r(&sec, &tmpTm);
    strftime(buffer, bufSize, format, &tmpTm);
    return buffer;
  }

#ifdef _XOPEN_SOURCE
  static TimeVal Parse(const char * date, const char * format, TimeVal defaultTime = 0) {
    char * end = NULL;
    tm tm;
    memset(&tm, 0, sizeof(tm));

    // fail of match returns NULL
    end = strptime(date, format, &tm);
    return (end != NULL) ? mktime(&tm) : defaultTime;
  }
#endif
};


class MilliscondTimer {
 public:
  MilliscondTimer() {
    Reset();
  }

  void Set(Time::Millisecond ms = 0) {
    begin_ = Time::GetMillisecond();
    end_ = begin_ + ms;
  }

  void Reset() {
    begin_ = 0;
    end_ = 0;
  }

  Time::Millisecond Get() {
    return Time::GetMillisecond() - begin_;
  }

  bool Expired() {
    return end_ < Time::GetMillisecond();
  }

 private:
  Time::Millisecond begin_;
  Time::Millisecond end_;
};

class MicroscondTimer {
 public:
  MicroscondTimer() {
    Reset();
  }

  void Set(Time::Microsecond ms = 0) {
    begin_ = Time::GetMicrosecond();
    end_ = begin_ + ms;
  }

  void Reset() {
    begin_ = 0;
    end_ = 0;
  }

  Time::Microsecond Get() {
    return Time::GetMicrosecond() - begin_;
  }

  bool Expired() {
    return end_ < Time::GetMicrosecond();
  }

 private:
  Time::Microsecond begin_;
  Time::Microsecond end_;
};

} //namespace minotaur

#endif //_MINOTAUR_COMMON_TIME_UTIL_H_
