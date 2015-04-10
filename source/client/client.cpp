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

namespace minotaur {

LOGGER_CLASS_IMPL(logger, Client);

Client::Client(
    IOService* io_service,
    const std::string& address,
    uint32_t timeout_ms) 
    : io_service_(io_service)
    , address_(address) 
    , timeout_ms_(timeout_ms)
    , channel_(NULL) {
}

Client::~Client() {
  Stop();
}

int Client::Start() {
  ClientChannel* channel = IODescriptorFactory::Instance()
    .CreateClientChannel(io_service_, address_, timeout_ms_);
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
     << "}" << std::endl;
}

bool Client::Send(ProtocolMessage* message) {
  if (GetStatus() != kWorking) {
    MI_LOG_TRACE(logger, "Client::Send Channel Broken" << *this);
    return false;
  }

  message->descriptor_id = channel_->GetDescriptorId();
  message->direction = ProtocolMessage::kOutgoingRequest;
  return coro::Send(message);
}

ProtocolMessage* Client::DoSendRecieve(ProtocolMessage* message, uint32_t timeout_ms) {
  if (GetStatus() != kWorking) {
    MI_LOG_TRACE(logger, "Client::Send Channel Broken" << *this);
    return NULL;   
  }

  message->descriptor_id = channel_->GetDescriptorId();
  message->direction = ProtocolMessage::kOutgoingRequest;
  return coro::SendRecieve(message, timeout_ms);
}

std::ostream& operator << (std::ostream& os, const Client& client) {
  client.Dump(os);
  return os;
}

} //namespace minotaur
