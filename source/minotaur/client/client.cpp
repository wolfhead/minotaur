/**
 * @file client.cpp
 * @author Wolfhead/
 */
#include "client.h"
#include "../coroutine/coro_all.h"
#include "../net/io_descriptor_factory.h"
#include "../net/client_channel.h"
#include "../service/service_handler.h"
#include "../net/io_descriptor_factory.h"
#include "client_sequence_generator.h"
#include "client_manager.h"

namespace ade {

LOGGER_CLASS_IMPL(logger, Client);

Client::Client(
    IOService* io_service,
    const std::string& name,
    const std::string& address,
    uint32_t timeout_ms,
    uint32_t heartbeat_ms) 
    : io_service_(io_service)
    , name_(name)
    , address_(address) 
    , timeout_ms_(timeout_ms)
    , heartbeat_ms_(heartbeat_ms)
    , channel_(NULL) {
}

Client::~Client() {
  Stop();
}

int Client::Start() {
  ClientChannel* channel = IODescriptorFactory::Instance()
    .CreateClientChannel(io_service_, address_, timeout_ms_, heartbeat_ms_);
  if (!channel) {
    MI_LOG_ERROR(logger, "Client::Start CreateClientChannel fail"
        << ", address:" << address_);
    return -1;
  }

  if (0 != channel->Start()) {
    MI_LOG_ERROR(logger, "Client::Start fail"
        << ", address:" << address_);
    IODescriptorFactory::Instance().Destroy(channel);
    return -1;
  }

  channel_ = channel;
  return 0;
}

int Client::Stop() {
  if (!channel_) {
    return 0;
  }

  int ret = channel_->Stop();
  channel_ = NULL;
  return ret;
}

void Client::Dump(std::ostream& os) const {
  os << "{\"type\": \"Client\""
     << ", \"address\": \"" << address_ << "\""
     << ", \"timeout\": " << timeout_ms_
     << ", \"heartbeat\": " << heartbeat_ms_
     << ", \"status\": \"" << ClientChannel::GetStatusString(GetChannelStatus()) << "\""
     << "}" << std::endl;
}

bool Client::Send(ProtocolMessage* message) {
  if (GetStatus() != kWorking) {
    MI_LOG_DEBUG(logger, "Client::Send Channel Broken" << *this);
    MessageFactory::Destroy(message);
    return false;
  }

  message->sequence_id = ClientSequenceGenerator::Generate(coro::CurrentHandlerId() + 1);
  message->descriptor_id = channel_->GetDescriptorId();
  message->direction = ProtocolMessage::kOneway;
  return coro::Send(message);
}

ProtocolMessage* Client::DoSendRecieve(ProtocolMessage* message, uint32_t timeout_ms) {
  if (GetStatus() != kWorking) {
    MI_LOG_DEBUG(logger, "Client::Send Channel Broken" << *this);
    MessageFactory::Destroy(message);
    return NULL;   
  }

  message->sequence_id = ClientSequenceGenerator::Generate(coro::CurrentHandlerId() + 1);
  message->descriptor_id = channel_->GetDescriptorId();
  message->direction = ProtocolMessage::kOutgoingRequest;
  coro::SetWaitingSequenceId(message->sequence_id);
  ProtocolMessage* response = coro::SendRecieve(message, timeout_ms ? timeout_ms : ClientManager::DefaultTimeout());
  if (response && response->status != ProtocolMessage::kStatusOK) { 
    MessageFactory::Destroy(response);
    return NULL;
  }

  return response;
}

std::ostream& operator << (std::ostream& os, const Client& client) {
  client.Dump(os);
  return os;
}

} //namespace ade
