/**
 * @file redis_protocol.cpp
 * @author Wolfhead
 */
#include "redis_protocol.h"
#include "redis_parser.h"
#include "../../io_buffer.h"
#include "../../io_redis_message.h"
#include "../../../matrix/matrix_mem_perf.h"

namespace ade {

LOGGER_CLASS_IMPL_NAME(logger, RedisProtocol, "net.RedisProtocol");

RedisProtocol::RedisProtocol() 
    : Protocol(ProtocolType::kRedisProtocol, true) 
    , current_(NULL) {
}

RedisProtocol::~RedisProtocol() {
  if (current_) {
    MessageFactory::Destroy(current_);
    current_ = NULL;
  }
}

ProtocolMessage* RedisProtocol::Decode(IOBuffer* buffer, int* result, ProtocolMessage* hint) {
  if (!hint) {
    MI_LOG_ERROR(logger, "RedisProtocol::Decode no hint, broken channel!, buffer:"
        << buffer->GetCStyle());
    *result = Protocol::kDecodeFail;
    return NULL;
  }

  MI_LOG_TRACE(logger, "RedisIncoming:" << buffer->GetCStyle());
  const char* incoming = buffer->GetCStyle();

  RedisRequestMessage* redis_request = (RedisRequestMessage*)hint;

  if (!current_) {
    current_ = MessageFactory::Allocate<RedisResponseMessage>(redis_request->command_count);
  }

  Slice s(buffer->GetCStyle(), buffer->GetReadSize());
  while (!current_->FinishAll()) {
    int ret = redis::RedisParser::Parse(s, current_->CurrentRESP());
    if (ret < 0) {
      *result = Protocol::kDecodeFail;
      return NULL;
    } else if (ret == 0) {
      *result = Protocol::kDecodeContinue;
      return NULL;
    } else {
      s.trim(ret);
      buffer->Consume(ret);
      current_->FinishOne();
    }
  }

  RedisResponseMessage* message = current_;
  *result = Protocol::kDecodeSuccess;
  current_ = NULL;

  MI_LOG_TRACE(logger, "RedisProtocol::Decode" << *message);

  if (message != NULL && buffer->GetReadSize()) {
    MI_LOG_WARN(logger, "RedisProtocol::Decode sync break" 
        << ", message:" << *message
        << ", incoming:" << incoming 
        << ", buffer:" << buffer->GetRead());
    MessageFactory::Destroy(message);
    message = NULL;
    *result = Protocol::kDecodeFail;
  }

  return message;
}

int RedisProtocol::Encode(IOBuffer* buffer, ProtocolMessage* message) {
  if (buffer->GetReadSize()) {
    MI_LOG_WARN(logger, "RedisProtocol::Encode sync break:" << buffer->GetRead());
  }

  RedisRequestMessage* redis_message = (RedisRequestMessage*)message;
  buffer->WriteString(redis_message->body); 
  LOG_TRACE(logger, "RedisProtocol " << buffer->GetCStyle());
  return Protocol::kEncodeSuccess;
}

ProtocolMessage* RedisProtocol::HeartBeatRequest() {
  RedisRequestMessage* message = MessageFactory::Allocate<RedisRequestMessage>();
  message->type_id = MessageType::kHeartBeatMessage;
  message->Ping();
  return message;
}

ProtocolMessage* RedisProtocol::HeartBeatResponse(ProtocolMessage* request) {
  return NULL;
}

void RedisProtocol::Reset() {
  if (current_) {
    MessageFactory::Destroy(current_);
    current_ = NULL;
  }
}

} //namespace ade 
