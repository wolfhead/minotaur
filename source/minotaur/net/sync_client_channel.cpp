/**
 * @file sync_client_channel.cpp
 * @author Wolfhead
 */
#include "sync_client_channel.h"
#include "protocol/protocol.h"
#include "io_handler.h"
#include "../io_service.h"
#include "../stage.h"
#include "../service/service_handler.h"

namespace ade { 

LOGGER_CLASS_IMPL(logger, SyncClientChannel);

SyncClientChannel::SyncClientChannel(
    IOService* io_service,
    uint32_t timeout_msec,
    uint32_t heartbeat_msec) 
    : ClientChannel(io_service, timeout_msec, heartbeat_msec)
    , sequence_keeper_(timeout_msec) {
}

int SyncClientChannel::EncodeMessage(ProtocolMessage* message) {
  if (GetStatus() != kConnected) { 
    MI_LOG_DEBUG(logger, "SyncClientChannel::EncodeMessage ChannelBroken:" << GetStatus());
    return -1;
  }

  if (0 != sequence_keeper_.Register(message)) {
    MI_LOG_WARN(logger, "SyncClientChannel::EncodeMessage Register fail");
    return -1;
  }

  TryFireMessage();
  return 0; 
}

void SyncClientChannel::OnDecodeMessage(ProtocolMessage* message) {
  ProtocolMessage* keeper_message = sequence_keeper_.Fetch();
  if (!keeper_message) {
    MI_LOG_WARN(logger, "SyncClientChannel::OnDecodeMessage keeper not found, might timeout"
        << ", client_channel:" << GetDiagnositicInfo());
    MessageFactory::Destroy(message);
    BreakChannel();
    return;
  } 

  if (keeper_message->type_id == MessageType::kHeartBeatMessage
      || keeper_message->direction == ProtocolMessage::kOneway) {
    MI_LOG_TRACE(logger, "SyncClientChannel::OnDecodeMessage heartbeat or oneway:" << *message);
    MessageFactory::Destroy(message);
    MessageFactory::Destroy(keeper_message);
    TryFireMessage();
    return;
  }

  message->status = ProtocolMessage::kStatusOK;
  message->direction = ProtocolMessage::kIncomingResponse;
  message->handler_id = keeper_message->handler_id;
  message->sequence_id = keeper_message->sequence_id;
  message->descriptor_id = GetDescriptorId();
  message->payload.data = keeper_message->payload.data;

  MI_LOG_TRACE(logger, "SyncClientChannel::OnDecodeMessage " << *message);

  MessageFactory::Destroy(keeper_message);

  if (!GetIOService()->GetServiceStage()->Send(message)) {
    MI_LOG_WARN(logger, "SyncClientChannel::OnDecodeMessage Send message fail");
    MessageFactory::Destroy(message);
    return;
  }

  TryFireMessage();
}

void SyncClientChannel::OnTimeout() {
  SyncSequenceKeeper::QueueType timeout_queue = sequence_keeper_.Timeout();
  if (timeout_queue.size()) {
    LOG_DEBUG(logger, "SyncClientChannel::Break, info:" << GetDiagnositicInfo());
    BreakChannel();
  }

  ProtocolMessage* message = timeout_queue.front();
  while (message) {
    timeout_queue.pop_front();

    MI_LOG_DEBUG(logger, "SyncClientChannel::OnTimeout, message:");
    DoSendBack(message, ProtocolMessage::kStatusTimeout);

    message = timeout_queue.front(); 
  }

  StartTimeoutTimer();
}

void SyncClientChannel::PurgeSequenceKeeper() {
  SyncSequenceKeeper::QueueType timeout_queue = sequence_keeper_.Clear();
  ProtocolMessage* message = timeout_queue.front();
  while (message) {
    timeout_queue.pop_front();

    DoSendBack(message, ProtocolMessage::kInternalFailure);

    message = timeout_queue.front(); 
  }
}

void SyncClientChannel::TryFireMessage() {
  while (true) {
    ProtocolMessage* fire_message = sequence_keeper_.Fire();
    if (!fire_message) {
      return;
    }

    if (Protocol::kEncodeSuccess != GetProtocol()->Encode(&write_buffer_, fire_message)) {
      MI_LOG_WARN(logger, "SyncClientChannel::TryFireMessage encode fail");
      DoSendBack(fire_message, ProtocolMessage::kStatusEncodeFail);
      sequence_keeper_.Fetch();
      continue;
    }

    OnWrite();
    break;
  }
}

int SyncClientChannel::DecodeMessage() {
  int result = Protocol::kDecodeSuccess;
  while (result == Protocol::kDecodeSuccess 
      && read_buffer_.GetReadSize()
      && sequence_keeper_.Fired()) {
    ProtocolMessage* message = GetProtocol()->Decode(
        &read_buffer_, 
        &result, 
        sequence_keeper_.Fired());
    if (!message) {
      continue;
    }

    OnDecodeMessage(message);
  }

  if (result == Protocol::kDecodeFail) {
    LOG_ERROR(logger, "SyncClientChannel::DecodeMessage fail, Broken");
    SetStatus(ClientChannel::kBroken);
    return -1;
  }

  return 0;
}

} //namespace ade
