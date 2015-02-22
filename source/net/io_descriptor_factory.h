#ifndef _MINOTAUR_IO_DESCRIPTOR_FACTORY_H_
#define _MINOTAUR_IO_DESCRIPTOR_FACTORY_H_
/**
 * @file io_descriptor_factory.h
 * @author Wolfhead
 */
#include "../common/logger.h"
#include "../lockfree/freelist.hpp"
#include "protocol/protocol_factory.h"

namespace minotaur {

class IOService;
class IODescriptor;
class Channel;
class Acceptor;
class Connector;

class IODescriptorFactory {
 public:
  static IODescriptorFactory& Instance() {
    static IODescriptorFactory instance_;
    return instance_;
  }


  Channel* CreateChannel(
      IOService* io_service, 
      int fd);

  Acceptor* CreateAcceptor(
      IOService* io_service,
      const std::string& host, 
      int port, 
      int protocol_type);

  Connector* CreateConnector(
      IOService* io_service,
      const std::string& host, 
      int port, 
      int protocol_type);

  static IODescriptor* GetIODescriptor(uint64_t descriptor_id) {
    return lockfree::freelist<IODescriptor>::get_key(descriptor_id);
  }

  bool Destroy(IODescriptor* descriptor);

 private:
  IODescriptorFactory();

  LOGGER_CLASS_DECL(logger);

  lockfree::freelist<IODescriptor> freelist_;
  ProtocolFactory protocol_factory_;
};

} //namespace minotaur

#endif //_MINOTAUR_IO_DESCRIPTOR_FACTORY_H_
