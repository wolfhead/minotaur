#ifndef MINOTAUR_CLIENT_MANAGER_H
#define MINOTAUR_CLIENT_MANAGER_H
/**
 * @file client_manager.h
 * @author Wolfhead
 */
#include "../common/logger.h"
#include "client_router.h"

namespace ade {

class IOService;

struct ClientConfig {
  std::string address;
  uint32_t timeout;
  uint32_t heartbeat;
  int count;
};

struct ClientRouterConfig {
  std::string name;
  std::string router;
  std::vector<ClientConfig> clients;
};

class ClientManager {
 public:
  static void SetDefaultTimeout(uint32_t timeout) {default_timeout_ = timeout;}  
  static uint32_t DefaultTimeout()  {return default_timeout_;}

  ClientManager(IOService* io_service);
  ~ClientManager();

  int Start();
  int Stop();

  int AddClient(const ClientRouterConfig& config);

  ClientRouter* GetClientRouter(const std::string& name) const;

  IOService* GetIOService() const {return io_service_;}

  std::string Diagnostic() const;

 private:
  LOGGER_CLASS_DECL(logger);

  IOService* io_service_;
  std::map<std::string, ClientRouter*> router_;

  static uint32_t default_timeout_;
};

} //namespace ade

#endif // MINOTAUR_CLIENT_MANAGER_H
