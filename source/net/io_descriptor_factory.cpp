/**
 * @file io_descriptor_factory.cpp
 * @author Wolfhead
 */
#include "io_descriptor_factory.h"
#include "channel.h"
#include "acceptor.h"
#include "connector.h"

namespace minotaur {

namespace {

struct io_descriptor_size {
  union {
    char channel_size[sizeof(Channel)];
    char acceptor_size[sizeof(Acceptor)];
    char cnnector_size[sizeof(Connector)];
  } data;
  uint64_t padding;
};

} //namespace 

LOGGER_CLASS_IMPL_NAME(logger, IODescriptorFactory, "net.IODescriptorFactory");

IODescriptorFactory::IODescriptorFactory() 
    : freelist_(1024, sizeof(io_descriptor_size)) 
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

Connector* IODescriptorFactory::CreateConnector(
    IOService* io_service,
    const std::string& host, 
    int port,
    int protocol_type) {
  Protocol* protocol = protocol_factory_.Create(protocol_type);
  if (!protocol) {
    MI_LOG_ERROR(logger, "IODescriptorFactory::CreateConnector create protocol fail:"
        << protocol_type);
    return NULL;
  }

  uint64_t descriptor_id = 0;
  Connector* connector = freelist_.alloc_with<Connector>(
      io_service, host, port, &descriptor_id);
  if (!connector) {
    return NULL;
  }

  connector->SetDescriptorId(descriptor_id);
  connector->SetProtocol(protocol);
  return connector;  
}

bool IODescriptorFactory::Destroy(IODescriptor* descriptor) {
  return freelist_.destroy(descriptor);
}

} //namespace minotaur
