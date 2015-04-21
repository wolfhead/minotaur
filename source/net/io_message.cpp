/**
 * @file io_message.cpp
 * @author Wolfhead
 */
#include "io_message.h"
#include <sstream>
#include "protocol/http/http_protocol.h"
#include "../common/macro.h"
#include "../3rd-party/http_parser/http_parser.h"

namespace minotaur {

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

  os << "}";
}

const std::string& HttpMessage::GetMethodString() const {
  return HttpProtocol::GetMethodString(method);
}

const std::string& HttpMessage::GetStatusString() const {
  return HttpProtocol::GetStatusString(status_code);
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


} //namespace minotaur
