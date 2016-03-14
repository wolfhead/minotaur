/**
 * @file online_debug.cpp
 * @author Wolfhead
 */
#include "online_debug.h"
#include <log4cplus/spi/factory.h>
#include <log4cplus/spi/loggerfactory.h>
#include <log4cplus/hierarchy.h>
#include "online_debug_logger_appender.h"
#include "online_debug_logger_factory.h"

namespace ade { namespace online_debug {

#define DEBUG_TRACE_SIZE (128)

bool OnlineDebug::init_ = false;
thread_local uint64_t g_debug_status_ = 0;

int OnlineDebug::Init() {
  if (init_) {
    return 0;
  }

  log4cplus::spi::AppenderFactoryRegistry& reg = 
    log4cplus::spi::getAppenderFactoryRegistry();
  LOG4CPLUS_REG_APPENDER (reg, OnlineDebugLoggerAppender);
 
  std::auto_ptr<log4cplus::spi::LoggerFactory> factory(new log4cplus::OnlineDebugLoggerFactory);
  log4cplus::Logger::getDefaultHierarchy().setLoggerFactory(factory);
  
  init_ = true;

  return 0;
}
 
void OnlineDebug::SetOnlineDebug(uint64_t status) {
  g_debug_status_ = status;
}

uint64_t OnlineDebug::GetOnlineDebug() { 
  return g_debug_status_;
}


 
} //namespace online_debug
} //namespace zmt
