#ifndef _MINOTAUR_UNITTEST_LOGGER_H_
#define _MINOTAUR_UNITTEST_LOGGER_H_

#include <string>
#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>
#include <log4cplus/helpers/timehelper.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/layout.h> 

namespace minotaur { namespace unittest {

class UnittestLogger {
 public:
  UnittestLogger(int level = log4cplus::ALL_LOG_LEVEL) {
    log4cplus::SharedAppenderPtr appender (new log4cplus::ConsoleAppender());
    //log4cplus::SharedAppenderPtr appender (new log4cplus::FileAppender("./log.log"));
    std::string pattern = "%D{[%Y/%m/%d-%H:%M:%S]} (%t) %-5p%x - %m [%l]%n";

    std::auto_ptr<log4cplus::Layout> layout(new log4cplus::PatternLayout(pattern));
    appender->setLayout(layout);

    log4cplus::Logger logger = log4cplus::Logger::getRoot();

    logger.addAppender(appender);

    logger.setLogLevel(level);  
  }

  UnittestLogger(const char * configFile) {
    log4cplus::PropertyConfigurator::doConfigure(configFile);
  }

  ~UnittestLogger() {
  }
};

}//namespace unittest
}//namespace minotaur

#endif // _MINOTAUR_UNITTEST_LOGGER_H_
