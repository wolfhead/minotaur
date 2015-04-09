/**
 * @file client.cpp
 * @author Wolfhead/
 */
#include "client.h"
#include "../net/io_descriptor_factory.h"

namespace minotaur {

LOGGER_CLASS_IMPL(logger, Client);

Client::Client(
    IOService* io_service,
    const std::string& address,
    uint32_t timeout_ms) 
    : io_service_(io_service)
    , address_(address) 
    , timeout_ms_(timeout_ms) {
}

int Client::Start() {
  ClientChannnel* channel = IODescriptorFactory::Instance()
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

  return channel_->Stop();
}

} //namespace minotaur
