#ifndef _MINOTAUR_IO_DESCRIPTOR_FACTORY_H_
#define _MINOTAUR_IO_DESCRIPTOR_FACTORY_H_
/**
 * @file io_descriptor_factory.h
 * @author Wolfhead
 */
#include "../common/logger.h"
#include "../lockfree/freelist.hpp"

namespace minotaur {

class IOService;
class IODescriptor;
class Channel;
class Acceptor;

class IODescriptorFactory {
 public:
  IODescriptorFactory(IOService* io_service);

  Channel* CreateChannel(int fd, const std::string& ip, int port);

  Acceptor* CreateAcceptor(const std::string& host, int port);

  IODescriptor* GetIODescriptor(uint64_t descriptor_id);

  bool Destroy(IODescriptor* descriptor);

 private:
  LOGGER_CLASS_DECL(logger);

  IOService* io_service_;
  lockfree::freelist<IODescriptor> freelist_;
};

} //namespace minotaur

#endif //_MINOTAUR_IO_DESCRIPTOR_FACTORY_H_
