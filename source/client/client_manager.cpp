/**
 * @file client_manager.cpp
 * @author Wolfhead
 */
#include "client_manager.h"

namespace minotaur {

LOGGER_CLASS_IMPL(logger, ClientManager);

ClientManager::ClientManager(IOService* io_service) 
    : io_service_(io_service) {
}

ClientManager::~ClientManager() {
  Stop();
  for (auto& pair : router_) {
    delete pair.second;
  } 
}

int ClientManager::Start() {
  for (auto& pair : router_) {
    if (0 != pair.second->Start()) {
      return -1;
    }
  }

  return 0;
}

int ClientManager::Stop() {
  for (auto& pair : router_) {
    pair.second->Stop();
  }

  return 0;
}

int ClientManager::AddClient(
    const std::string& name, 
    const std::string& address, 
    uint32_t timeout_ms, 
    uint32_t heartbeat_ms) {
  ClientRouter* client_router = NULL;
  if (router_.find(name) != router_.end()) {
    client_router = router_[name];
  } else {
    client_router = new ClientRouter(GetIOService());
    router_[name] = client_router;
  }

  return client_router->AddClient(address, timeout_ms, heartbeat_ms);
}

ClientRouter* ClientManager::GetClientRouter(const std::string& name) {
  auto it = router_.find(name);
  if (it != router_.end()) {
    return it->second;
  } else {
    return NULL;
  }
}

} //namespace minotaur
