/**
 * @file memcached_protocol.cpp
 * @author Wolfhead
 */
#include "memcached_protocol.h"
#include <string.h>
#include "../../io_buffer.h"
#include "../../io_message.h"
#include "../../../common/string_util.h"

namespace ade {

LOGGER_CLASS_IMPL_NAME(logger, MemcachedProtocol, "net.MemcachedProtocol");

MemcachedProtocol::MemcachedProtocol() 
    : Protocol(ProtocolType::kLineProtocol, true)
    , current_(NULL) {
}

MemcachedProtocol::~MemcachedProtocol() {
  if (current_) {
    delete current_;
    current_ = NULL;
  }
}

ProtocolMessage* MemcachedProtocol::Decode(
      IOBuffer* buffer, 
      int* result,
      ProtocolMessage* /*hint*/) {
  static const std::string kValue = "VALUE";
  static const std::string kEnd = "END";

  if (!current_) {
    current_ = MessageFactory::Allocate<LineMessage>("");
  }

  LineMessage* message = NULL;

  while (buffer->GetReadSize()) {
    buffer->EnsureCStyle();
    char* begin = buffer->GetRead();
    char* end = strstr(begin, "\r\n");

    if (!end) {
      *result = Protocol::kDecodeContinue;
      return NULL;
    }

    buffer->Consume(end - begin + 2);
    std::string data(begin, end - begin);

    if (strutil::StartsWith(data, kValue)) {
      continue;
    }

    if (strutil::StartsWith(data, kEnd)) {
      *result = Protocol::kDecodeSuccess;
      message = current_;
      current_ = NULL;
      if (buffer->GetReadSize()) {
        LOG_WARN(logger, "MemcachedProtocol::Decode sync break:" << buffer->GetRead());
        MessageFactory::Destroy(message);
        message = NULL;
        *result = Protocol::kDecodeFail;
      }
      return message;
    }

    current_->body = data;
  }

  *result = Protocol::kDecodeContinue;
  return NULL;
}

int MemcachedProtocol::Encode(
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

ProtocolMessage* MemcachedProtocol::HeartBeatRequest() {
  return NULL;
}

ProtocolMessage* MemcachedProtocol::HeartBeatResponse(ProtocolMessage* request) {
  return NULL;
}

void MemcachedProtocol::Reset() {
  if (current_) {
    MessageFactory::Destroy(current_);
    current_ = NULL;
  }
}

} //namespace ade

