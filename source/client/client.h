#ifndef _MINOTAUR_CLIENT_CLIENT_H_
#define _MINOTAUR_CLIENT_CLIENT_H_
/**
 * @file client.h
 * @author Wolfhead
 */
#include <string>
#include "../common/logger.h"

namespace minotaur {

class IOService;
class ProtocolMessage;

class Client {
 public:
  Client(
      IOService* io_service, 
      const std::string& address, 
      uint32_t timeout_ms);

  int Start();

  int Stop();

  bool Send(ProtocolMessage* message);

  ProtocolMessage* SendRecieve(ProtocolMessage* message, uint32_t timeout);

 private:
  LOGGER_CLASS_DECL(logger);

  IOService* io_service_;
  std::string address_;
  uint32_t timeout_ms_; 
};

} //namespace minotaur

#endif //_MINOTAUR_CLIENT_CLIENT_H_  
