#ifndef _MINOTAUR_CLIENT_CLIENT_H_
#define _MINOTAUR_CLIENT_CLIENT_H_
/**
 * @file client.h
 * @author Wolfhead
 */
#include <string>
#include "../common/logger.h"
#include "../net/client_channel.h"
#include "../net/io_message.h"

namespace ade {

class IOService;
class ClientChannel;

class Client {
 public:
  enum {
    kWorking = 0,
    kBroken = 1,
  };

  Client(
      IOService* io_service, 
      const std::string& name,
      const std::string& address, 
      uint32_t timeout_ms,
      uint32_t heartbeat_ms);

  ~Client();

  inline const std::string& GetName() const {return name_;}

  int Start();

  int Stop();

  bool Send(ProtocolMessage* message);

  template<typename T>
  typename T::ResponseType * SendRecieve(T* message, uint32_t timeout_ms = 0) {
    return (typename T::ResponseType*)DoSendRecieve(message, timeout_ms);
  }

  int GetStatus() const {
    return channel_ && channel_->GetStatus() == ClientChannel::kConnected ? kWorking : kBroken;
  }

  int GetChannelStatus() const {
    return channel_ ? channel_->GetStatus() : -1;
  }

  IOService* GetIOService() const {return io_service_;}

  const std::string& GetAddress() const {return address_;}

  uint32_t GetTimeout() const {return timeout_ms_;}

  uint32_t GetHeartBeat() const {return heartbeat_ms_;}

  void Dump(std::ostream& os) const;

 private:
  LOGGER_CLASS_DECL(logger);

  ProtocolMessage* DoSendRecieve(ProtocolMessage* message, uint32_t timeout_ms = 0);

  IOService* io_service_;
  std::string name_;
  std::string address_;
  uint32_t timeout_ms_; 
  uint32_t heartbeat_ms_;

  ClientChannel* channel_;
};

std::ostream& operator << (std::ostream& os, const Client& client);

} //namespace ade

#endif //_MINOTAUR_CLIENT_CLIENT_H_  
