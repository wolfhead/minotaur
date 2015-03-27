#ifndef _MINOTAUR_IO_DESCRIPTOR_FACTORY_H_
#define _MINOTAUR_IO_DESCRIPTOR_FACTORY_H_
/**
 * @file io_descriptor_factory.h
 * @author Wolfhead
 */
#include "../common/logger.h"
#include "../lockfree/freelist.hpp"
#include "io_descriptor.h"
#include "protocol/protocol_factory.h"

namespace minotaur {

class IOService;
class Channel;
class ServiceChannel;
class Acceptor;
class ClientChannel;
class Service;

class IODescriptorFactory {
 public:
  static IODescriptorFactory& Instance() {
    static IODescriptorFactory instance_;
    return instance_;
  }

  static IODescriptor* GetIODescriptor(uint64_t descriptor_id) {
    return lockfree::freelist<IODescriptor>::get_key(descriptor_id);
  }

  static uint8_t GetVersion(uint64_t descriptor_id) {
    return lockfree::tagged_ptr<IODescriptor>
        ::extract_tag(descriptor_id);
  }

  static int ParseAddress(
      const std::string& address, 
      std::string* ip, 
      int* port, 
      int* protocol);

  ServiceChannel* CreateServiceChannel(
      IOService* io_service, 
      int fd,
      Service* service);

  Acceptor* CreateAcceptor(
      IOService* io_service,
      const std::string& address,
      const std::string& service_name = "");

  ClientChannel* CreateClientChannel(
      IOService* io_service,
      const std::string& address,
      uint32_t timeout_msec);

  bool Destroy(IODescriptor* descriptor);

 private:
  IODescriptorFactory();

  LOGGER_CLASS_DECL(logger);

  lockfree::freelist<IODescriptor> freelist_;
  ProtocolFactory protocol_factory_;
};

} //namespace minotaur

#endif //_MINOTAUR_IO_DESCRIPTOR_FACTORY_H_
