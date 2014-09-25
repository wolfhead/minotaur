#Find and set LOG4CPLUS_INCLUDE_DIRS, LOG4CPLUS_LIBRARIES

find_path(LOG4CPLUS_INCLUDE_DIRS log4cplus/logger.h)
find_library(LOG4CPLUS_LIBRARIES NAMES liblog4cplus.a log4cplus)

message(STATUS "log4cplus include: " ${LOG4CPLUS_INCLUDE_DIRS})
message(STATUS "log4cplus library: " ${LOG4CPLUS_LIBRARIES})
