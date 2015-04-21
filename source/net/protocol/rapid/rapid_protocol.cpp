/**
 * @file rapid_protocol.cpp
 * @author Wolfhead
 */
#include "rapid_protocol.h"
#include <string.h>
#include "../../io_buffer.h"
#include "../../io_message.h"

namespace minotaur {

namespace {
struct RapidHeader {
  uint32_t size; // total package length in bytes
  uint32_t seqid;
  uint8_t  type; // 
  uint8_t  version;
  uint16_t cmdid;
  uint32_t extra;
};

} //namespace

RapidProtocol::RapidProtocol() 
    : Protocol(ProtocolType::kRapidProtocol, false) {
}

ProtocolMessage* RapidProtocol::Decode(
      IOBuffer* buffer, 
      int* result) {
  if (buffer->GetReadSize() < sizeof(RapidHeader)) {
    *result = Protocol::kDecodeContinue;
    return NULL;
  }

  RapidHeader* header = (RapidHeader*)buffer->GetRead();
  if (buffer->GetReadSize() < header->size) {
    *result = Protocol::kDecodeContinue;
    return NULL;
  }

  RapidMessage* message = MessageFactory::Allocate<RapidMessage>();

  if (header->type == RapidMessage::kHeartBeatType) {
    message->type_id = MessageType::kHeartBeatMessage;
  }

  message->sequence_id = header->seqid;
  message->cmd_id = header->cmdid;
  message->extra = header->extra;
  message->body.assign(
      buffer->GetRead() + sizeof(RapidHeader), 
      header->size - sizeof(RapidHeader));
  
  buffer->Consume(header->size);
  *result = Protocol::kDecodeSuccess;
  return message;
}

int RapidProtocol::Encode(
      IOBuffer* buffer,
      ProtocolMessage* message) {
  RapidMessage* rapid_message = 
    static_cast<RapidMessage*>(message);

  uint32_t package_size = 
      sizeof(RapidHeader) + rapid_message->body.size();

  uint8_t message_type = RapidMessage::kDataType;
  if (message->type_id == MessageType::kHeartBeatMessage) {
    message_type = RapidMessage::kHeartBeatType;
  }

  RapidHeader header = {
    .size = package_size,
    .seqid = rapid_message->sequence_id,
    .type = message_type,
    .version = 0,
    .cmdid = rapid_message->cmd_id,
    .extra = rapid_message->extra,
  };

  buffer->Write((const char*)&header, sizeof(RapidHeader));
  buffer->Write(rapid_message->body.data(), rapid_message->body.size());
  return Protocol::kEncodeSuccess;
}

ProtocolMessage* RapidProtocol::HeartBeatRequest() {
  RapidMessage* rapid_message = MessageFactory::Allocate<RapidMessage>();
  rapid_message->type_id = MessageType::kHeartBeatMessage;
  return rapid_message;
}

ProtocolMessage* RapidProtocol::HeartBeatResponse(ProtocolMessage* message) {
  return message;
}

} //namespace minotaur
