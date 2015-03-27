#ifndef _MINOTAUR_NET_SERVICE_CHANNEL_H_
#define _MINOTAUR_NET_SERVICE_CHANNEL_H_
/**
 * @file service_channel.h
 * @author Wolfhead
 */

#include "channel.h"

namespace minotaur {

class Service;

class ServiceChannel : public Channel {
 public:
  ServiceChannel(IOService* io_service, int fd, Service* service);

  inline Service* GetService() {return service_;}

 protected:

  virtual void OnDecodeMessage(ProtocolMessage* message);

 private:
  LOGGER_CLASS_DECL(logger);

  Service* service_;
};

} //namespace minotaur

#endif //_MINOTAUR_NET_SERVICE_CHANNEL_H_
