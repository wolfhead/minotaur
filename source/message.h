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
    kUnknownEvent = 0,
    kIOReadEvent,
    kIOWriteEvent,
    kIOCloseEvent,
  };

  static const char* ToString(int type) {
    static const char* str[] = {
      "io read event",
      "io write event",
      "io close event",
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

class IOMessage : public MessageBase {
 public:
  IOMessage() 
      : MessageBase(MessageType::kUnknownEvent), descriptor_id(0) {
  }

  IOMessage(uint8_t type_id_, uint64_t descriptor_id_) 
      : MessageBase(type_id_), descriptor_id(descriptor_id_) {
  }

  uint64_t descriptor_id;
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
