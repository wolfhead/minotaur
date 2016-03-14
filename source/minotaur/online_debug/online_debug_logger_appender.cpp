/**
 * @file online_debug_logger_appender.h
 * @author Wolfhead
 */
#include "online_debug_logger_appender.h"
#include "online_debug.h"
#include "../common/macro.h"
#include <log4cplus/spi/loggingevent.h>
 
namespace log4cplus {
 
OnlineDebugLoggerAppender::OnlineDebugLoggerAppender(
    const log4cplus::helpers::Properties& properties) 
    : RollingFileAppender(properties) {
  
}
 
OnlineDebugLoggerAppender::~OnlineDebugLoggerAppender() {
  destructorImpl();
}
 
void OnlineDebugLoggerAppender::close() {
}
 
void OnlineDebugLoggerAppender::append(const spi::InternalLoggingEvent& event) {
  if (LIKELY(0 == ade::online_debug::OnlineDebug::GetOnlineDebug())) {
    return;
  }

  RollingFileAppender::append(event);
}
 
} //namespace log4cplus
