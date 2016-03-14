/**
 * @file service.cpp
 * @author Wolfhead
 */
#include "service.h"

namespace ade {

LOGGER_CLASS_IMPL(logger, ServiceFactory);

ServiceFactory::ServiceFactory() {
}

ServiceFactory::~ServiceFactory() {
  for (auto& pair : services_) {
    ADE_DELETE(pair.second);
  }
}

int ServiceFactory::RegisterService(const std::string& name, Service* service) {
  service->SetName(name);

  auto it = services_.find(name);
  if (it != services_.end()) {
    LOG_FATAL(logger, "ServiceFactory::RegisterService duplicate name:" << name);
    return -1;
  }
  services_.insert(std::make_pair(name, service));
  return 0;
}

Service* ServiceFactory::GetService(const std::string& name) {
  const auto it = services_.find(name);
  if (it == services_.end())  {
    return NULL;
  }
  return it->second;
}

} //namespace ade
