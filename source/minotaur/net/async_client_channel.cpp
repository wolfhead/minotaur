/**
 * @file async_client_channel.cpp
 * @author Wolfhead
 */
#include "async_client_channel.h"
#include "protocol/protocol.h"
#include "io_handler.h"
#include "../io_service.h"
#include "../stage.h"
#include "../service/service_handler.h"

namespace ade {

LOGGER_CLASS_IMPL(logger, AsyncClientChannel);

AsyncClientChannel::AsyncClientChannel(
    IOService* io_service,
    uint32_t timeout_msec,
    uint32_t heartbeat_msec) 
    : ClientChannel(io_service, timeout_msec, heartbeat_msec)
    , sequence_keeper_(timeout_msec) {
}

int AsyncClientChannel::EncodeMessage(ProtocolMessage* message) {
  if (GetStatus() != kConnected) {
    MI_LOG_DEBUG(logger, "AsyncClientChannel::EncodeMessage ChannelBroken:" << GetStatus());
    return -1;
  }

  if (Protocol::kEncodeSuccess != GetProtocol()->Encode(&write_buffer_, message)) {
    MI_LOG_ERROR(logger, "AsyncClientChannel::EncodeMessage fail");
    return -1;
  }

  if (0 != sequence_keeper_.Register(message)) {
    MI_LOG_WARN(logger, "AsyncClientChannel::OnEncodeMessage Register fail");
    return -1;
  }

  return 0;
}

void AsyncClientChannel::OnDecodeMessage(ProtocolMessage* message) {
  ProtocolMessage* keeper_message = sequence_keeper_.Fetch(message->sequence_id);
  if (!keeper_message) {
    MI_LOG_DEBUG(logger, "AsyncClientChannel::OnDecodeMessage sequence not found" 
        << ", sequence_id:" << message->sequence_id
        << ", client_channel:" << GetDiagnositicInfo());
    MessageFactory::Destroy(message);
    return;
  }

  if (keeper_message->type_id == MessageType::kHeartBeatMessage
      || keeper_message->direction == ProtocolMessage::kOneway) {
    MI_LOG_TRACE(logger, "AsyncClientChannel::OnDecodeMessage heartbeat:" << *message);
    MessageFactory::Destroy(message);
    MessageFactory::Destroy(keeper_message);
    return;
  }

  message->status = ProtocolMessage::kStatusOK;
  message->direction = ProtocolMessage::kIncomingResponse;
  message->handler_id = keeper_message->handler_id;
  message->sequence_id = keeper_message->sequence_id;
  message->descriptor_id = GetDescriptorId();
  message->payload.data = keeper_message->payload.data;
  message->next_ = NULL;

  MI_LOG_TRACE(logger, "AsyncClientChannel::OnDecodeMessage " << *message);

  MessageFactory::Destroy(keeper_message);

  if (!GetIOService()->GetServiceStage()->Send(message)) {
    MI_LOG_WARN(logger, "AsyncClientChannel::OnDecodeMessage Send message fail");
    MessageFactory::Destroy(message);
    return;
  }
}

void AsyncClientChannel::OnTimeout() {
  AsyncSequenceKeeper::QueueType timeout_queue = sequence_keeper_.Timeout();
  ProtocolMessage* message = timeout_queue.front();
  while (message) {
    timeout_queue.pop_front();
    if (message->type_id == MessageType::kHeartBeatMessage) {
      MI_LOG_DEBUG(logger, "AsyncClientChannel::OnTimeout break");
      BreakChannel();
    } else {
      MI_LOG_DEBUG(logger, "AsyncClientChannel::OnTimeout message");
    }
    DoSendBack(message, ProtocolMessage::kStatusTimeout);
    message = timeout_queue.front(); 
  }

  StartTimeoutTimer();
}

void AsyncClientChannel::PurgeSequenceKeeper() {
  AsyncSequenceKeeper::QueueType queue = sequence_keeper_.Clear();
  ProtocolMessage* message = queue.front();
  while (message) {
    queue.pop_front();
    DoSendBack(message, ProtocolMessage::kInternalFailure);
    message = queue.front(); 
  }
}

} //namespace ade
