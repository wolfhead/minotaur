#ifndef _MINOTAUR_PROTOCOL_FACTORY_H_
#define _MINOTAUR_PROTOCOL_FACTORY_H_
/**
 * @file protocol_factory.h
 * @author Wolfhead
 */
#include "../../common/logger.h"

namespace minotaur {

class Protocol;

class ProtocolType {
 public:
  enum {
    kUnknownProtocol = 0,
    kLineProtocol,
    kRapidProtocol,
    kHttpProtocol,
  };

  static const std::string& ToString(int type) {
    static const std::string str[] = {
      "unknown",
      "line",
      "rapid",
      "http",
    };

    if (type < 0 || type >= (int)(sizeof(str) / sizeof(std::string))) {
      type = 0;
    }

    return str[type];
  }

  static int ToType(const std::string& s) {
    static const std::string str[] = {
      "unknown",
      "line",
      "rapid",
      "http"
    };

    for (size_t i = 0; i != sizeof(str) / sizeof(std::string); ++i) {
      if (str[i] == s) {
        return i;
      }
    }
    return 0;
  }
};

class ProtocolFactory {
 public:
  ProtocolFactory();

  ~ProtocolFactory();

  Protocol* Create(int type);

 private:
  LOGGER_CLASS_DECL(logger);

  void RegisterProtocol(int type, Protocol* protocol);

  std::vector<Protocol*> protocol_;
};

} //namespace minotaur

#endif //_MINOTAUR_PROTOCOL_FACTORY_H_
