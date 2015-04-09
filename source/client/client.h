#ifndef _MINOTAUR_CLIENT_CLIENT_H_
#define _MINOTAUR_CLIENT_CLIENT_H_
/**
 * @file client.h
 * @author Wolfhead
 */
#include <string>
#include "../common/logger.h"
#include "../net/client_channel.h"

namespace minotaur {

class IOService;
class ClientChannel;
class ProtocolMessage;

class Client {
 public:
  enum {
    kWorking = 0,
    kBroken = 1,
  };

  Client(
      IOService* io_service, 
      const std::string& address, 
      uint32_t timeout_ms);

  ~Client();

  int Start();

  int Stop();

  bool Send(ProtocolMessage* message);

  template<typename T>
  T* SendRecieve(T* message, uint32_t timeout_ms = 0) {
    return (T*)DoSendRecieve(message, timeout_ms);
  }

  int GetStatus() {
    return channel_ && channel_->GetStatus() == ClientChannel::kConnected ? kWorking : kBroken;
  }

  IOService* GetIOService() const {return io_service_;}

  const std::string& GetAddress() const {return address_;}

  uint32_t GetTimeout() const {return timeout_ms_;}

  void Dump(std::ostream& os);

 private:
  LOGGER_CLASS_DECL(logger);

  ProtocolMessage* DoSendRecieve(ProtocolMessage* message, uint32_t timeout_ms = 0);

  IOService* io_service_;
  std::string address_;
  uint32_t timeout_ms_; 

  ClientChannel* channel_;
};

} //namespace minotaur

#endif //_MINOTAUR_CLIENT_CLIENT_H_  
