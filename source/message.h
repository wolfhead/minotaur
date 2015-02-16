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
      : type_id_(type_id) {
  }

  virtual ~MessageBase() {
  };

  uint8_t type_id() const {return type_id_;}

  uint32_t channel_id() const {return channel_id_;}
  void channel_id(uint32_t channel_id) {channel_id_ = channel_id;}

 private:
  uint8_t type_id_;
  uint32_t channel_id_;
};

class MessageFactory {
 public:
  template<typename MessageType>
  static MessageType* Allocate() {
    return new MessageType();
  }

  static void Destory(MessageBase* message) {
    delete message;
  }
};

} //namespace minotaur

#endif // _MINOTAUR_MESSAGE_H_
