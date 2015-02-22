/**
 * @file io_message.cpp
 * @author Wolfhead
 */
#include "io_message.h"
#include <sstream>

namespace minotaur {

///////////////////////////////////////////////////////////////////////
// LineMessage
void ProtocolMessage::Dump(std::ostream& os) const {
  os << "{\"type\": \"ProtocolMessage\""
     << ", \"type_id\": " << (int)type_id
     << ", \"descriptor_id\": " << descriptor_id
     << ", \"sequence_id\": " << sequence_id
     << "}";
}


///////////////////////////////////////////////////////////////////////
// LineMessage
void LineProtocolMessage::Dump(std::ostream& os) const {
  os << "{\"type\": \"LineProtocolMessage\""
     << ", \"type_id\": " << (int)type_id
     << ", \"descriptor_id\": " << descriptor_id
     << ", \"body\": \"" << body << "\""
     << "}";
}



///////////////////////////////////////////////////////////////////////
// HttpMessage
void HttpProtocolMessage::Dump(std::ostream& os) const {
  os << "{\"type\": \"HttpProtocolMessage\""
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

std::ostream& operator << (std::ostream& os, const LineProtocolMessage& message) {
  message.Dump(os);
  return os;
}

std::ostream& operator << (std::ostream& os, const HttpProtocolMessage& message) {
  message.Dump(os);
  return os;
}


} //namespace minotaur
