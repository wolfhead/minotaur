#ifndef MINOTAUR_CLIENT_MANAGER_H
#define MINOTAUR_CLIENT_MANAGER_H
/**
 * @file client_manager.h
 * @author Wolfhead
 */
#include "../common/logger.h"
#include "client_router.h"

namespace minotaur {

class IOService;

class ClientManager {
 public:
  ClientManager(IOService* io_service);
  ~ClientManager();

  int Start();
  int Stop();

  int AddClient(
      const std::string& name, 
      const std::string& address, 
      uint32_t timeout_ms,
      uint32_t heartbeat_ms);

  ClientRouter* GetClientRouter(const std::string& name);

  IOService* GetIOService() const {return io_service_;}

 private:
  LOGGER_CLASS_DECL(logger);

  IOService* io_service_;
  std::map<std::string, ClientRouter*> router_;
};

} //namespace minotaur

#endif // MINOTAUR_CLIENT_MANAGER_H
