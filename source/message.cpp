/**
 * @file message.cpp
 * @author Wolfhead
 */
#include "message.h"
#include "net/io_message.h"

namespace minotaur {

std::string MessageBase::ToString() const {
  std::ostringstream oss;
  Dump(oss);
  return oss.str();
}

void MessageBase::Dump(std::ostream& os) const {
  os << "{\"type\": \"IOMessage\""
     << ", \"type_id\": " << type_id << "}";
};

///////////////////////////////////////////////////////////////////////
// EventMessage
void EventMessage::Destroy() const {
  if (GetProtocolMessage()) {
    MessageFactory::Destroy(GetProtocolMessage());
  }
}

void EventMessage::Dump(std::ostream& os) const {
  os << "{\"type\": \"EventMessage\""
     << ", \"type_id\": " << (int)type_id
     << ", \"descriptor_id\": " << descriptor_id;
  if (GetProtocolMessage()) {
    os << ", \"payload\": " << GetProtocolMessage()->ToString();
  } else {
    os << ", \"payload\": " << payload;
  }
  os << "}";
}

std::ostream& operator << (std::ostream& os, const EventMessage& message) {
  message.Dump(os);
  return os;
}

} //namespace minotaur
