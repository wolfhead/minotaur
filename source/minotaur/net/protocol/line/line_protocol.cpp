/**
 * @file line_protocol.cpp
 * @author Wolfhead
 */
#include "line_protocol.h"
#include <string.h>
#include "../../io_buffer.h"
#include "../../io_message.h"

namespace ade {

LOGGER_CLASS_IMPL_NAME(logger, LineProtocol, "net.LineProtocol");

LineProtocol::LineProtocol() 
    : Protocol(ProtocolType::kLineProtocol, true) {
}

ProtocolMessage* LineProtocol::Decode(
      IOBuffer* buffer, 
      int* result,
      ProtocolMessage* /*hint*/) {

  buffer->EnsureCStyle();
  char* begin = buffer->GetRead();
  char* end = strstr(begin, "\r\n");

  if (!end) {
    *result = Protocol::kDecodeContinue;
    return NULL;
  }

  buffer->Consume(end - begin + 2);
  *result = Protocol::kDecodeSuccess;
  LineMessage* message = MessageFactory::Allocate<LineMessage>(
      std::string(begin, end - begin));
  return message;
}

int LineProtocol::Encode(
      IOBuffer* buffer,
      ProtocolMessage* message) {
  LineMessage* line_message = static_cast<LineMessage*>(message);
  const std::string& body = line_message->body;

  char* begin = buffer->EnsureWrite(body.size() + 2);

  memcpy(begin, body.data(), body.size());
  *(begin + body.size()) = '\r';
  *(begin + body.size() + 1) = '\n';

  buffer->Produce(body.size() + 2);
  return Protocol::kEncodeSuccess;
}

ProtocolMessage* LineProtocol::HeartBeatRequest() {
  return NULL;
}

ProtocolMessage* LineProtocol::HeartBeatResponse(ProtocolMessage* request) {
  return NULL;
}

} //namespace ade
