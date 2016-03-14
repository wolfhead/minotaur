#ifndef MINOTAUR_ONLINE_DEBUG_LOGGER_FACTORY_H
#define MINOTAUR_ONLINE_DEBUG_LOGGER_FACTORY_H
/**
 * @file online_debug_logger_factory.h
 * @author Wolfhead
 */
#include <log4cplus/spi/loggerfactory.h>
#include <log4cplus/spi/loggerimpl.h>
#include <log4cplus/logger.h>
#include "online_debug.h"
#include "../common/macro.h"

namespace log4cplus {

class OnlineDebugLoggerImpl : public spi::LoggerImpl {
 public:
  OnlineDebugLoggerImpl(const log4cplus::tstring& name, Hierarchy& h) 
      : spi::LoggerImpl(name, h) {
  }


  virtual bool isEnabledFor(LogLevel ll) const { 
    if (UNLIKELY(0 != ade::online_debug::OnlineDebug::GetOnlineDebug())) {
      return true;
    }
    return spi::LoggerImpl::isEnabledFor(ll);
  }
};

class OnlineDebugLoggerFactory : public DefaultLoggerFactory {
 public:
  Logger makeNewLoggerInstance(const log4cplus::tstring& name, Hierarchy& h) {
    return Logger(new OnlineDebugLoggerImpl (name, h)); 
  }
};

} //namespace log4cplus

#endif // MINOTAUR_ONLINE_DEBUG_LOGGER_FACTORY_H
