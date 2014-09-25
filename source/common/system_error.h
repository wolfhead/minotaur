#ifndef _MINOTAUR_COMMON_SYSTEM_ERROR_H_
#define _MINOTAUR_COMMON_SYSTEM_ERROR_H_
/**
 * @file system_error.h
 * @author Wolfhead
 *
 * define helper class to encapsulate system error
 * handling and error message formatting
 */
#include <errno.h>
#include <string.h>
#include <string>
#include <sstream>

namespace minotaur {

class SystemError {
 public:
  inline static int Get() {
    return errno; 
  }

  //TODO
  //refactoring later with optimization
  inline static std::string ToString() {
    char buffer[512];
    return strerror_r(errno, buffer, 512);
  }

  inline static std::string FormatMessage() {
    std::ostringstream oss;
    oss << "[" << Get() << "] " << ToString();
    return oss.str();
  }  
};

} //namespace minotaur

#endif // _MINOTAUR_COMMON_SYSTEM_ERROR_H_
