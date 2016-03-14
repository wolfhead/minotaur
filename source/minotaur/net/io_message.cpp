/**
 * @file io_message.cpp
 * @author Wolfhead
 */
#include "io_message.h"
#include <sstream>
#include "protocol/http/http_protocol.h"
#include "protocol/redis/redis_parser.h"
#include "../common/macro.h"
#include "../3rd-party/http_parser/http_parser.h"

namespace ade {

const std::string HttpMessage::kUserAgent = "User-Agent";
const std::string HttpMessage::kReferer = "Referer";
const std::string HttpMessage::kEmptyString = "";

///////////////////////////////////////////////////////////////////////
// ProtocolMessage
void ProtocolMessage::Dump(std::ostream& os) const {
  os << "{\"type\": \"ProtocolMessage\""
     << ", \"type_id\": " << (int)type_id
     << ", \"descriptor_id\": " << descriptor_id
     << ", \"status\": " << status
     << ", \"direction\": " << (int)direction
     << ", \"handler_id\": " << handler_id
     << ", \"sequence_id\": " << sequence_id
     << ", \"payload\": " << payload.data
     << ", \"time\": " << time.data
     << ", \"next_\": " << next_
     << ", \"prev_\": " << prev_
     << "}";
}


///////////////////////////////////////////////////////////////////////
// LineMessage
void LineMessage::Dump(std::ostream& os) const {
  os << "{\"type\": \"LineMessage\""
     << ", \"ProtocolMessage\": ";
  ProtocolMessage::Dump(os);
  os << ", \"body\": \"" << body << "\""
     << "}";
}

///////////////////////////////////////////////////////////////////////
// RapidMessage
void RapidMessage::Dump(std::ostream& os) const {
  os << "{\"type\": \"RapidMessage\""
     << ", \"ProtocolMessage\": ";
  ProtocolMessage::Dump(os);   
  os << ", \"cmd_id\": " << cmd_id
     << ", \"extra\": " << extra
     << ", \"body\": \"" << body << "\""
     << "}";
}


///////////////////////////////////////////////////////////////////////
// HttpMessage
void HttpMessage::Dump(std::ostream& os) const {
  os << "{\"type\": \"HttpMessage\""
     << ", \"ProtocolMessage\": ";
  ProtocolMessage::Dump(os);      
  os << ", \"http_major\": " << (int)http_major
     << ", \"http_minor\": " << (int)http_minor
     << ", \"status_code\": " << (int)status_code
     << ", \"method\": " << (int)method
     << ", \"keep_alive\": " << keep_alive
     << ", \"url\": \"" << url << "\""
     << ", \"body\": \"" << body << "\"";

  for (const auto& pair : header) {
    os << ", \"header." << pair.first << "\": \"" << pair.second << "\"";
  }

  for (const auto& cookie : set_cookies) {
    os<<", \"header.Set-Cookie\": \""<< cookie << "\"";
  }

  os << "}";
}

HttpMessage* HttpMessage::AsResponse(int code, const std::string& _body) {
  direction = ProtocolMessage::kOutgoingResponse;
  status_code = code;
  body = _body;
  header.clear();
  return this;
}

const std::string& HttpMessage::GetMethodString() const {
  return HttpProtocol::GetMethodString(method);
}

const std::string& HttpMessage::GetStatusString() const {
  return HttpProtocol::GetStatusString(status_code);
}

void HttpMessage::GetUrlEntity(HttpUrlEntity* entity) const {
  SliceHttpUrlEntity slice_entity;
  HttpProtocol::ParseUrl(Slice(url), &slice_entity);
  entity->BuildFromSlice(slice_entity);
}

void HttpMessage::GetSliceUrlEntity(SliceHttpUrlEntity* entity) const {
  HttpProtocol::ParseUrl(Slice(url), entity);
}

void HttpMessage::GetCookie(std::map<std::string, std::string>* cookie) const {
  static const std::string cookie_field = "Cookie";
  const auto it = header.find(cookie_field);
  if (it == header.end()) {
    return;
  }

  std::map<Slice, Slice> slice_cookie;
  HttpProtocol::ParseCookie(Slice(it->second), &slice_cookie);
  for (auto& pair : slice_cookie) {
    cookie->insert(std::make_pair(pair.first.str(), pair.second.str()));
  }
}

std::ostream& operator << (std::ostream& os, const ProtocolMessage& message) {
  message.Dump(os);
  return os;
}

std::ostream& operator << (std::ostream& os, const LineMessage& message) {
  message.Dump(os);
  return os;
}

std::ostream& operator << (std::ostream& os, const RapidMessage& message) {
  message.Dump(os);
  return os;
}

std::ostream& operator << (std::ostream& os, const HttpMessage& message) {
  message.Dump(os);
  return os;
}


} //namespace ade
