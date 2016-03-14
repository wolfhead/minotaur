#ifndef _MINOTAUR_COMMON_LOGGER_H_
#define _MINOTAUR_COMMON_LOGGER_H_

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>
#include <log4cplus/helpers/timehelper.h>
#include "../online_debug/online_debug.h"

#ifndef NO_ONLINE_DEBUG
  #define INIT_ONLINE_DEBUGGER(line) \
      ade::online_debug::OnlineDebug::Init()
  #define MINOTAUR_GET_ONLINE_DEBUG() \
      ade::online_debug::OnlineDebug::GetOnlineDebug()
  #define MINOTAUR_SET_ONLINE_DEBUG(status) \
      ade::online_debug::OnlineDebug::SetOnlineDebug(status)

  #define HAS_ONLINE_DEBUG
#else
  #define INIT_ONLINE_DEBUGGER() (0)
  #define MINOTAUR_GET_ONLINE_DEBUG() (0)
  #define MINOTAUR_SET_ONLINE_DEBUG(status)
#endif

#define LOGGER_SYS_INIT(property_file) \
    log4cplus::PropertyConfigurator::doConfigure(property_file);
#define LOGGER_SYS_INIT_DYNAMIC(watcher, property_file, check_interval_ms)  \
    log4cplus::ConfigureAndWatchThread watcher(property_file, check_interval_ms);

#define LOGGER_CLASS_DECL(logger) \
    static log4cplus::Logger logger;
#define LOGGER_CLASS_IMPL(logger, classname) \
    log4cplus::Logger classname::logger = (INIT_ONLINE_DEBUGGER(), log4cplus::Logger::getInstance(#classname));
#define LOGGER_CLASS_IMPL_NAME(logger, classname, name) \
    log4cplus::Logger classname::logger = (INIT_ONLINE_DEBUGGER(), log4cplus::Logger::getInstance(name));

#define LOGGER_EXTERN_DECL(logger) \
    extern  log4cplus::Logger logger;
#define LOGGER_IMPL(logger, name)  \
    log4cplus::Logger logger = (INIT_ONLINE_DEBUGGER(), log4cplus::Logger::getInstance(name));

#define LOGGER_STATIC_DECL_IMPL(logger,name) \
    static log4cplus::Logger logger = (INIT_ONLINE_DEBUGGER(), log4cplus::Logger::getInstance(name));


#define LOG_TRACE(logger,e)  LOG4CPLUS_TRACE(logger,e)
#define LOG_DEBUG(logger,e) LOG4CPLUS_DEBUG(logger,e)
#define LOG_INFO(logger,e) LOG4CPLUS_INFO(logger,e)
#define LOG_WARN(logger,e) LOG4CPLUS_WARN(logger,e)
#define LOG_ERROR(logger,e)  LOG4CPLUS_ERROR(logger,e)
#define LOG_FATAL(logger,e) LOG4CPLUS_FATAL(logger,e)

#define MI_LOG_TRACE(logger, e) LOG_TRACE(logger, e)
#define MI_LOG_DEBUG(logger, e) LOG_DEBUG(logger, e)
#define MI_LOG_INFO(logger, e) LOG_INFO(logger, e)
#define MI_LOG_WARN(logger, e) LOG_WARN(logger, e)
#define MI_LOG_ERROR(logger, e) LOG_ERROR(logger, e)
#define MI_LOG_FATAL(logger, e) LOG_FATAL(logger, e)

#endif // _MINOTAUR_COMMON_LOGGER_H_
