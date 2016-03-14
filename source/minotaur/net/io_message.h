#ifndef _MINOTAUR_IO_MESSAGE_H_
#define _MINOTAUR_IO_MESSAGE_H_
/**
 * @file io_message.h
 * @author Wolfhead
 */
#include <string>
#include <map>
#include <functional>
#include "../message.h"
#include <vector>

namespace ade {

class Service;
class HttpUrlEntity;
class SliceHttpUrlEntity;

class ProtocolMessage : public MessageBase {
 public:
  typedef ProtocolMessage ResponseType;

  enum {
    kDirectionUnknown = 0,
    kIncomingRequest,
    kOutgoingRequest,
    kIncomingResponse,
    kOutgoingResponse,
    kOneway,
  };

  enum {
    kStatusOK = 0,
    kStatusTimeout,
    kStatusEncodeFail,
    kInternalFailure,
  };

  ProtocolMessage() : next_(NULL), prev_(NULL) {}

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

  std::string ip;
  int port;

  // for book-keeper
  ProtocolMessage* next_;
  ProtocolMessage* prev_;
};

class LineMessage : public ProtocolMessage {
 public:
  typedef LineMessage ResponseType;

  LineMessage(const std::string& body_)
      : body(body_) {
    type_id = MessageType::kLineMessage;
  }

  virtual void Dump(std::ostream& os) const;

  std::string body;
};

class RapidMessage : public ProtocolMessage {
 public:
  typedef RapidMessage ResponseType;

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
  static const std::string kUserAgent;
  static const std::string kReferer;
  static const std::string kEmptyString;

  typedef HttpMessage ResponseType;

  HttpMessage() {
    type_id = MessageType::kHttpMessage;
    gzip = false;
  }

  HttpMessage* AsResponse(int code, const std::string& body);

  const std::string& GetMethodString() const;

  const std::string& GetStatusString() const;

  void GetUrlEntity(HttpUrlEntity* entity) const;

  void GetSliceUrlEntity(SliceHttpUrlEntity* entity) const;

  void SetCookie(const std::string& set_cookie) {
    set_cookies.push_back(set_cookie);
  }

  void GetCookie(std::map<std::string, std::string>* cookie) const;

  inline const std::string& GetUserAgent() const {
    const auto it = header.find(kUserAgent);
    if (it == header.end()) {
      return kEmptyString;
    } 
    return it->second;
  }
  inline const std::string& GetReferer() const {
    const auto it = header.find(kReferer);
    if (it == header.end()) {
      return kEmptyString;
    } 
    return it->second;
  }

  virtual void Dump(std::ostream& os) const;

  unsigned short http_major;
  unsigned short http_minor;
  unsigned short status_code;
  unsigned short method;
  bool keep_alive; 
  bool gzip;

  std::string url;
  std::map<std::string, std::string> header;
  std::vector<std::string> set_cookies;
  std::string body;
};

// this is some kind of hack
// TimerMessage will be fired from ServiceTimerThread to ServiceTimerStage
class TimerMessage : public ProtocolMessage {
 public:
  typedef std::function<void()> TimerProc;

  TimerMessage(const TimerProc& _proc) 
      : proc(_proc) {
    type_id = MessageType::kTimerMessage;
  }

  TimerProc proc;
};

std::ostream& operator << (std::ostream& os, const ProtocolMessage& message);
std::ostream& operator << (std::ostream& os, const LineMessage& message);
std::ostream& operator << (std::ostream& os, const RapidMessage& message);
std::ostream& operator << (std::ostream& os, const HttpMessage& message);


} //namespace ade

#endif //_MINOTAUR_IO_MESSAGE_H_
