/**
 * @file service.cpp
 * @author Wolfhead
 */
#include "service.h"

namespace minotaur {

LOGGER_CLASS_IMPL(logger, ServiceManager);

ServiceManager::ServiceManager() {
}

ServiceManager::~ServiceManager() {
  for (auto& pair : services_) {
    delete pair.second;
  }
}

int ServiceManager::RegisterService(const std::string& name, Service* service) {
  auto it = services_.find(name);
  if (it != services_.end()) {
    LOG_FATAL(logger, "ServiceManager::RegisterService duplicate name:" << name);
    return -1;
  }
  services_.insert(std::make_pair(name, service));
  return 0;
}

Service* ServiceManager::GetService(const std::string& name) {
  const auto it = services_.find(name);
  if (it == services_.end())  {
    return NULL;
  }
  return it->second;
}

} //namespace minotaur
