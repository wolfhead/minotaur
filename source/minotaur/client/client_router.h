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

namespace ade {

class IOService;

class ClientRouter {
 public:
  ClientRouter(IOService* io_service, const std::string& name);
  ~ClientRouter();

  inline const std::string& GetName() const {return name_;}

  int AddClient(const std::string& address, uint32_t timeout_ms, uint32_t heartbeat_ms);
  int AddClientRouter(ClientRouter* router);
  int Start();
  int Stop();

  bool Send(ProtocolMessage* message);
  bool SendHash(ProtocolMessage* message, uint32_t hash);

  template<typename T>
  typename T::ResponseType * SendRecieve(T* message, uint32_t timeout_ms = 0) {
    return (typename T::ResponseType*)DoSendRecieve(message, timeout_ms);
  }

  template<typename T>
  typename T::ResponseType* SendRecieveHash(T* message, uint32_t hash, uint32_t timeout_ms = 0) {
    return (typename T::ResponseType*)DoSendRecieveHash(message, hash, timeout_ms);
  }

  IOService* GetIOService() const {return io_service_;}

  std::string Diagnostic() const;

 private:
  LOGGER_CLASS_DECL(logger);

  ProtocolMessage* DoSendRecieve(ProtocolMessage* message, uint32_t timeout_ms = 0);
  ProtocolMessage* DoSendRecieveHash(ProtocolMessage* message, uint32_t hash, uint32_t timeout_ms = 0);

  Client* GetNextClient();
  ClientRouter* GetHashClient(uint32_t hash);

  IOService* io_service_;
  std::string name_;
  std::vector<Client*> clients_;
  std::vector<ClientRouter*> hash_;
  std::atomic<uint32_t> current_;
};

} //namespace ade

#endif // _MINOTAUR_CHANNEL_CLIENT_H_
