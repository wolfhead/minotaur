/**
 * @file io_descriptor_factory.cpp
 * @author Wolfhead
 */
#include "io_descriptor_factory.h"
#include "channel.h"
#include "acceptor.h"

namespace minotaur {

LOGGER_CLASS_IMPL_NAME(logger, IODescriptorFactory, "net.IODescriptorFactory");

IODescriptorFactory::IODescriptorFactory() 
    : freelist_(1024, 256) 
    , protocol_factory_() {
}

Channel* IODescriptorFactory::CreateChannel(
    IOService* io_service,
    int fd) {
  uint64_t descriptor_id = 0;
  Channel* channel = freelist_.alloc_with<Channel>(
      io_service, fd, &descriptor_id);
  if (!channel) {
    return NULL;
  }

  channel->SetDescriptorId(descriptor_id);
  return channel;
}

Acceptor* IODescriptorFactory::CreateAcceptor(
    IOService* io_service,
    const std::string& host, 
    int port,
    int protocol_type) {
  Protocol* protocol = protocol_factory_.Create(protocol_type);
  if (!protocol) {
    MI_LOG_ERROR(logger, "IODescriptorFactory::CreateAcceptor create protocol fail:"
        << protocol_type);
    return NULL;
  }

  uint64_t descriptor_id = 0;
  Acceptor* acceptor = freelist_.alloc_with<Acceptor>(
      io_service, host, port, &descriptor_id);
  if (!acceptor) {
    return NULL;
  }

  acceptor->SetDescriptorId(descriptor_id);
  acceptor->SetProtocol(protocol);
  return acceptor;  
}

IODescriptor* IODescriptorFactory::GetIODescriptor(uint64_t descriptor_id) {
  return freelist_.get_key(descriptor_id);
}

bool IODescriptorFactory::Destroy(IODescriptor* descriptor) {
  return freelist_.destroy(descriptor);
}

} //namespace minotaur
