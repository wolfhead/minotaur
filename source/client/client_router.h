#ifndef _MINOTAUR_CHANNEL_CLIENT_H_
#define _MINOTAUR_CHANNEL_CLIENT_H_
/**
 * @file channel_client.h
 * @author Wolfhead
 */
#include <vector>
#include <atomic>
#include "../common/logger.h"
#include "client.h"

namespace minotaur {

class IOService;

class ClientRouter {
 public:
  ClientRouter(IOService* io_service);
  ~ClientRouter();

  int AddClient(const std::string& address, uint32_t timeout_ms, uint32_t heartbeat_ms);
  int Start();
  int Stop();

  bool Send(ProtocolMessage* message);
  bool SendHash(ProtocolMessage* message, uint32_t hash);

  template<typename T>
  T* SendRecieve(T* message, uint32_t timeout_ms = 0) {
    return (T*)DoSendRecieve(message, timeout_ms);
  }

  template<typename T>
  T* SendRecieveiHash(T* message, uint32_t hash, uint32_t timeout_ms = 0) {
    return (T*)DoSendRecieve(message, hash, timeout_ms);
  }

  IOService* GetIOService() const {return io_service_;}

 private:
  LOGGER_CLASS_DECL(logger);

  ProtocolMessage* DoSendRecieve(ProtocolMessage* message, uint32_t timeout_ms = 0);
  ProtocolMessage* DoSendRecieveHash(ProtocolMessage* message, uint32_t hash, uint32_t timeout_ms = 0);

  Client* GetNextClient();
  Client* GetHashClient(uint32_t hash);

  IOService* io_service_;
  std::vector<Client*> clients_;
  std::atomic<uint32_t> current_;
};

} //namespace minotaur

#endif // _MINOTAUR_CHANNEL_CLIENT_H_
