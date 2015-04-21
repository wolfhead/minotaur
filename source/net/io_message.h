#ifndef _MINOTAUR_IO_MESSAGE_H_
#define _MINOTAUR_IO_MESSAGE_H_
/**
 * @file io_message.h
 * @author Wolfhead
 */
#include <string>
#include <map>
#include "../message.h"

namespace minotaur {

class Service;

class ProtocolMessage : public MessageBase {
 public:
  enum {
    kDirectionUnknown = 0,
    kIncomingRequest,
    kOutgoingRequest,
    kIncomingResponse,
    kOutgoingResponse,
  };

  enum {
    kStatusOK = 0,
    kStatusTimeout,
    kInternalFailure,
  };

  template<typename T>
  T GetPayloadAs() {
    return (T)(payload.data);
  }

  virtual void Dump(std::ostream& os) const;

  uint8_t status;
  uint8_t direction;
  uint16_t handler_id;
  uint32_t sequence_id;
  uint64_t descriptor_id;
  union {
    uint64_t data;
    uint64_t coroutine_id;
    Service* service;
  } payload;
  union {
    uint64_t data;
    uint64_t birthtime;
    uint64_t timeout;
  } time;
  // for book-keeper
  ProtocolMessage* next_;
  ProtocolMessage* prev_;
};

class LineMessage : public ProtocolMessage {
 public:
  LineMessage(const std::string& body_)
      : body(body_) {
    type_id = MessageType::kLineMessage;
  }

  virtual void Dump(std::ostream& os) const;

  std::string body;
};

class RapidMessage : public ProtocolMessage {
 public:
  enum {
    kDataType = 0,
    kHeartBeatType = 1,
    kOneway = 2,
  };

  RapidMessage() {
    type_id = MessageType::kRapidMessage;
  }

  virtual void Dump(std::ostream& os) const;

  uint16_t cmd_id;
  uint32_t extra;
  std::string body;
};

class HttpMessage : public ProtocolMessage {
 public:
  HttpMessage() {
    type_id = MessageType::kHttpMessage;
  }

  const std::string& GetMethodString() const;

  const std::string& GetStatusString() const;

  virtual void Dump(std::ostream& os) const;

  unsigned short http_major;
  unsigned short http_minor;
  unsigned short status_code;
  unsigned short method;
  bool keep_alive; 

  std::string url;
  std::map<std::string, std::string> header;
  std::string body;
};

std::ostream& operator << (std::ostream& os, const ProtocolMessage& message);
std::ostream& operator << (std::ostream& os, const LineMessage& message);
std::ostream& operator << (std::ostream& os, const RapidMessage& message);
std::ostream& operator << (std::ostream& os, const HttpMessage& message);


} //namespace minotaur

#endif //_MINOTAUR_IO_MESSAGE_H_
