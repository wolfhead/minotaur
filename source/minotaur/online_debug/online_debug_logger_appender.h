#ifndef MINOTAUR_ONLINE_DEBUG_LOGGER_APPENDER_H
#define MINOTAUR_ONLINE_DEBUG_LOGGER_APPENDER_H
#include <log4cplus/appender.h>
#include <log4cplus/fileappender.h>
/**
 * @file online_debug_logger_appender.h
 */

namespace log4cplus {

class OnlineDebugLoggerAppender : public RollingFileAppender {
 public:
  OnlineDebugLoggerAppender(const log4cplus::helpers::Properties& properties);

  virtual ~OnlineDebugLoggerAppender();

  virtual void close();

 protected:
  virtual void append(const spi::InternalLoggingEvent& event);
};

} //namespace log4cplus

#endif // MINOTAUR_ONLINE_DEBUG_LOGGER_APPENDER_H
