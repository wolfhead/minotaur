/**
 * @file io_message.cpp
 * @author Wolfhead
 */
#include "io_message.h"
#include <sstream>

namespace minotaur {

///////////////////////////////////////////////////////////////////////
// ProtocolMessage
void ProtocolMessage::Dump(std::ostream& os) const {
  os << "{\"type\": \"ProtocolMessage\""
     << ", \"type_id\": " << (int)type_id
     << ", \"descriptor_id\": " << descriptor_id
     << ", \"status\": " << status
     << ", \"direction\": " << direction
     << ", \"handler_id\": " << handler_id
     << ", \"sequence_id\": " << sequence_id
     << ", \"descriptor_id\": " << descriptor_id
     << ", \"payload\": " << payload
     << ", \"next_\": " << next_
     << ", \"prev_\": " << prev_
     << ", \"timestamp_\": " << timestamp_
     << "}";
}


///////////////////////////////////////////////////////////////////////
// LineMessage
void LineMessage::Dump(std::ostream& os) const {
  os << "{\"type\": \"LineMessage\""
     << ", \"type_id\": " << (int)type_id
     << ", \"descriptor_id\": " << descriptor_id
     << ", \"body\": \"" << body << "\""
     << "}";
}

///////////////////////////////////////////////////////////////////////
// RapidMessage
void RapidMessage::Dump(std::ostream& os) const {
  os << "{\"type\": \"RapidMessage\""
     << ", \"type_id\": " << (int)type_id
     << ", \"descriptor_id\": " << descriptor_id
     << ", \"cmd_id\": " << cmd_id
     << ", \"extra\": " << extra
     << ", \"body\": \"" << body << "\""
     << "}";
}


///////////////////////////////////////////////////////////////////////
// HttpMessage
void HttpMessage::Dump(std::ostream& os) const {
  os << "{\"type\": \"HttpMessage\""
     << ", \"type_id\": " << (int)type_id
     << ", \"descriptor_id\": " << descriptor_id
     << ", \"http_major\": " << http_major
     << ", \"http_minor\": " << http_minor
     << ", \"status_code\": " << status_code
     << ", \"method\": " << method
     << ", \"http_type\": " << http_type
     << ", \"keep_alive\": " << keep_alive
     << ", \"url\": \"" << url << "\""
     << ", \"body\": \"" << body << "\"";

  for (const auto& pair : header) {
    os << ", \"header." << pair.first << "\": \"" << pair.second << "\"";
  }

  os << "}";
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
