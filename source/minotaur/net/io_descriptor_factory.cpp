/**
 * @file io_descriptor_factory.cpp
 * @author Wolfhead
 */
#include "io_descriptor_factory.h"
#include "acceptor.h"
#include "channel.h"
#include "client_channel.h"
#include "async_client_channel.h"
#include "sync_client_channel.h"
#include "service_channel.h"
#include "socket_op.h"
#include "protocol/protocol.h"
#include "../service/service.h"

namespace ade {

namespace {

struct io_descriptor_size {
  union {
    char channel_size[sizeof(Channel)];
    char acceptor_size[sizeof(Acceptor)];
    char service_channel_size[sizeof(ServiceChannel)];
    char client_channel_size[sizeof(ClientChannel)];
    char async_client_channel_size[sizeof(AsyncClientChannel)];
    char sync_client_channel_size[sizeof(SyncClientChannel)];
  } data;
  uint64_t padding;
};

} //namespace 

LOGGER_CLASS_IMPL_NAME(logger, IODescriptorFactory, "net.IODescriptorFactory");

IODescriptorFactory::IODescriptorFactory() 
    : freelist_(1024, sizeof(io_descriptor_size)) 
    , protocol_factory_() {
}

ServiceChannel* IODescriptorFactory::CreateServiceChannel(
    IOService* io_service,
    int fd,
    Service* service) {
  uint64_t descriptor_id = 0;
  ServiceChannel* channel = freelist_.alloc_with<ServiceChannel>(
      &descriptor_id, io_service, fd, service);
  if (!channel) {
    return NULL;
  }

  channel->SetDescriptorId(descriptor_id);
  return channel;
}

Acceptor* IODescriptorFactory::CreateAcceptor(
    IOService* io_service, 
    const std::string& address,
    const std::string& service_name /* = ""*/) {
  std::string ip;
  int port = 0;
  int protocol_type = 0;

  if (0 != ParseAddress(address, &ip, &port, &protocol_type)) {
    MI_LOG_ERROR(logger, "IODescriptorFactory::CreateAcceptor ParseAddress fail"
        << ", address:" << address);
    return NULL;
  }

  Protocol* protocol = protocol_factory_.Create(protocol_type);
  if (!protocol) {
    MI_LOG_ERROR(logger, "IODescriptorFactory::CreateAcceptor unknown protocol:" << protocol_type);
    return NULL;
  }

  Service* service = ServiceFactory::Instance()->GetService(service_name);
  if (!service && !service_name.empty()) {
    MI_LOG_ERROR(logger, "IODescriptorFactory::CreateAcceptor unknown service:" << service_name);
    return NULL;
  }

  uint64_t descriptor_id = 0;
  Acceptor* acceptor = freelist_.alloc_with<Acceptor>(
      &descriptor_id, io_service, ip, port, service);
  if (!acceptor) {
    return NULL;
  }

  acceptor->SetDescriptorId(descriptor_id);
  acceptor->SetProtocol(protocol);
  return acceptor;  
}

ClientChannel* IODescriptorFactory::CreateClientChannel(
    IOService* io_service,
    const std::string& address,
    uint32_t timeout_msec,
    uint32_t heartbeat_msec) {
  std::string ip;
  int port = 0;
  int protocol_type = 0;

  if (0 != ParseAddress(address, &ip, &port, &protocol_type)) {
    MI_LOG_ERROR(logger, "IODescriptorFactory::CreateClientChannel ParseAddress fail"
        << ", address:" << address);
    return NULL;
  }

  Protocol* protocol = protocol_factory_.Create(protocol_type);
  if (!protocol) {
    MI_LOG_ERROR(logger, "IODescriptorFactory::CreateClientChannel unknown protocol:" << protocol_type);
    return NULL;
  }

  uint64_t descriptor_id = 0;
  ClientChannel* client_channel = NULL;

  if (protocol->IsSync()) {
    client_channel = freelist_.alloc_with<SyncClientChannel>(
      &descriptor_id, io_service, timeout_msec, heartbeat_msec);
  } else {
    client_channel = freelist_.alloc_with<AsyncClientChannel>(
      &descriptor_id, io_service, timeout_msec, heartbeat_msec);
  }

  if (!client_channel) {
    return NULL;
  }

  client_channel->SetIp(ip);
  client_channel->SetPort(port);
  client_channel->SetDescriptorId(descriptor_id);
  client_channel->SetProtocol(protocol);
  return client_channel;  
}

bool IODescriptorFactory::Destroy(IODescriptor* descriptor) {
  return freelist_.destroy(descriptor);
}

int IODescriptorFactory::ParseAddress(
    const std::string& address, 
    std::string* ip, 
    int* port, 
    int* protocol) {
  char proto[64] = {0};
  char host[256] = {0};
  int count = sscanf(address.c_str(), " %[^:]://%[^:]:%d", proto, host, port);
  if (count != 3) {
    MI_LOG_ERROR(logger, "IODescriptorFactory::ParseAddress fail:" << address);
    return -1;
  }

  ip->assign(SocketOperation::GetHostIP(host));
  if (ip->empty()) {
    MI_LOG_ERROR(logger, "IODescriptorFactory::ParseAddress unknown host:" << host);
    return -1;
  }

  *protocol = ProtocolType::ToType(proto);
  if (*protocol == ProtocolType::kUnknownProtocol) {
    MI_LOG_ERROR(logger, "IODescriptorFactory::ParseAddress unknown protocol:" << proto);
    return -1;
  }

  return 0;
}


} //namespace ade
