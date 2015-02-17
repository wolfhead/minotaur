#ifndef _MINOTAUR_MESSAGE_H_
#define _MINOTAUR_MESSAGE_H_
/**
 * @file message.h
 * @author Wolfhead
 */
#include <stdint.h>

namespace minotaur {

class MessageType {
 public:
  enum {
    kReadEvent = 1,
    kWriteEvent,
  };

  static const char* ToString(int type) {
    static const char* str[] = {
      "read event",
      "write event",
    };

    if (type < 0 || type >= (int)(sizeof(str) / sizeof(const char*))) {
      return "unknown event";
    }

    return str[type];
  }
};

class MessageBase {
 public:
  MessageBase(uint8_t type_id) 
      : type_id(type_id) {
  }
  virtual ~MessageBase() {};

  uint8_t type_id;
};

class IOMessageBase : public MessageBase {
 public:
  IOMessageBase(uint8_t type_id, uint64_t channel_id) 
      : MessageBase(type_id), channel_id(channel_id) {
  }

  uint32_t channel_id;
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

  static void Destory(MessageBase* message) {
    delete message;
  }
};

} //namespace minotaur

#endif // _MINOTAUR_MESSAGE_H_
