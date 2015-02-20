#ifndef _MINOTAUR_MESSAGE_H_
#define _MINOTAUR_MESSAGE_H_
/**
 * @file message.h
 * @author Wolfhead
 */
#include <stdint.h>
#include <sstream>

namespace minotaur {

class MessageType {
 public:
  enum {
    kUnknownEvent = 0,
    kIOEvent,
    kIOMessageEvent,
    kLineProtocolMessage,
    kHttpProtocolMessage,
  };

  static const char* ToString(int type) {
    static const char* str[] = {
      "unknown event",
      "io event",
      "io message event",
      "line protocol message",
      "Http protocol message",
    };

    if (type < 0 || type >= (int)(sizeof(str) / sizeof(const char*))) {
      return "unknown event";
    }

    return str[type];
  }
};

class MessageBase {
 public:
  MessageBase()
      : type_id(MessageType::kUnknownEvent) {
  }

  MessageBase(uint8_t type_id) 
      : type_id(type_id) {
  }
  virtual ~MessageBase() {};

  std::string ToString() const {
    std::ostringstream oss;
    Dump(oss);
    return oss.str();
  }

  virtual void Dump(std::ostream& os) const {
    os << "{\"type\": \"IOMessage\""
       << ", \"type_id\": " << type_id << "}";
  };

  uint8_t type_id;
};


class MessageFactory {
 public:
  template<typename MessageType>
  static MessageType* Allocate() {
    return new MessageType();
  }

  template<typename MessageType, typename A1>
  static MessageType* Allocate(const A1& arg1) {
    return new MessageType(arg1);
  }

  template<typename MessageType, typename A1, typename A2>
  static MessageType* Allocate(const A1& arg1, const A2& arg2) {
    return new MessageType(arg1, arg2);
  }

  static void Destroy(MessageBase* message) {
    delete message;
  }
};

} //namespace minotaur

#endif // _MINOTAUR_MESSAGE_H_
