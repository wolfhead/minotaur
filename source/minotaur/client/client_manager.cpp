/**
 * @file client_manager.cpp
 * @author Wolfhead
 */
#include "client_manager.h"

namespace ade {

LOGGER_CLASS_IMPL(logger, ClientManager);

uint32_t ClientManager::default_timeout_ = 0;

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

int ClientManager::AddClient(const ClientRouterConfig& config) {
  if (router_.find(config.name) != router_.end()) {
    LOG_ERROR(logger, "ClientManager::AddClient Dupldate client:" << config.name);
    return -1;
  }

  ClientRouter* router = new ClientRouter(io_service_, config.name);
  for (const auto& client_config : config.clients) {
    if (config.router == "hash") {
      ClientRouter* sub_router = new ClientRouter(io_service_, config.name);
      for (int i = 0; i != client_config.count; ++i) {
        if (0 != sub_router->AddClient(
              client_config.address, 
              client_config.timeout, 
              client_config.heartbeat)) {
          LOG_ERROR(logger, "ClientManager AddClient fail:" << client_config.address);
          delete router;
          delete sub_router;
          return -1;
        }
      }     
      router->AddClientRouter(sub_router);
    } else {
      for (int i = 0; i != client_config.count; ++i) {
        if (0 != router->AddClient(
              client_config.address, 
              client_config.timeout, 
              client_config.heartbeat)) {
          LOG_ERROR(logger, "ClientManager AddClient fail:" << client_config.address);
          delete router;
          return -1;
        }
      }
    }
  }

  router_[config.name] = router;
  return 0;
}

ClientRouter* ClientManager::GetClientRouter(const std::string& name) const {
  const auto it = router_.find(name);
  if (it != router_.end()) {
    return it->second;
  } else {
    return NULL;
  }
}

std::string ClientManager::Diagnostic() const {
  std::ostringstream oss;
  oss << "{";
  bool first = true;
  for (const auto& pair : router_) {
    if (first) {
      first = false;
    } else {
      oss << ", ";
    }
    oss << "\"" << pair.first << "\": " << pair.second->Diagnostic();
  }
  oss << "}";
  return oss.str();
}

} //namespace ade
