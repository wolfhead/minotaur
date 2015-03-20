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

class ProtocolMessage : public MessageBase {
 public:
  enum {
    kIncomingRequest = 0,
    kOutgoingRequest,
    kIncomingResponse,
    kOutgoingResponse,
  };

  enum {
    kStatusOK = 0,
    kInternalFailure,
  };

  virtual void Dump(std::ostream& os) const;

  uint8_t status;
  uint8_t direction;
  uint16_t reserve;
  uint32_t sequence_id;
  uint64_t descriptor_id;
  uint64_t payload;
  // for book-keeper
  ProtocolMessage* next_;
  ProtocolMessage* prev_;
  uint64_t timestamp_;
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
    kPingType = 1,
    kPongType = 2,
    kOneway = 3,
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
  enum {
    kMethodDelete = 0,
    kMethodGet = 1,
    kMethodHead = 2,
    kMethodPost = 3,
    kMethodPut = 4,
    kHttpTypeRequest = 0,
    kHttpTypeResponse = 1,
    kHttpTypeBoth = 2,
  };

  HttpMessage() {
    type_id = MessageType::kHttpMessage;
  }

  virtual void Dump(std::ostream& os) const;

  unsigned short http_major;
  unsigned short http_minor;
  unsigned short status_code;
  unsigned short method;
  unsigned short http_type;
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
