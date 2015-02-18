/**
 * @file io_descriptor_factory.cpp
 * @author Wolfhead
 */
#include "io_descriptor_factory.h"
#include "channel.h"
#include "acceptor.h"

namespace minotaur {

LOGGER_CLASS_IMPL_NAME(logger, IODescriptorFactory, "net.IODescriptorFactory");

IODescriptorFactory::IODescriptorFactory(IOService* io_service) 
    : io_service_(io_service) 
    , freelist_(1024, 256) {
}

Channel* IODescriptorFactory::CreateChannel(
    int fd, 
    const std::string& ip, 
    int port) {
  uint64_t descriptor_id = 0;
  Channel* channel = freelist_.alloc_with<Channel>(
      io_service_, fd, &descriptor_id);
  if (!channel) {
    return NULL;
  }

  channel->SetDescriptorId(descriptor_id);
  channel->SetIp(ip);
  channel->SetPort(port);

  return channel;
}

Acceptor* IODescriptorFactory::CreateAcceptor(
    const std::string& host, 
    int port) {
  uint64_t descriptor_id = 0;
  Acceptor* acceptor = freelist_.alloc_with<Acceptor>(
      io_service_, host, port, &descriptor_id);
  if (!acceptor) {
    return NULL;
  }

  acceptor->SetDescriptorId(descriptor_id);
  return acceptor;  
}

IODescriptor* IODescriptorFactory::GetIODescriptor(uint64_t descriptor_id) {
  return freelist_.get_key(descriptor_id);
}

bool IODescriptorFactory::Destroy(IODescriptor* descriptor) {
  return freelist_.destroy(descriptor);
}

} //namespace minotaur
