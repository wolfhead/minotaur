/**
 * @file config_manager.cpp
 * @author Wolfhead
 */
#include "config_manager.h"
#include "../common/xml_macro.h"

namespace ade {

LOGGER_CLASS_IMPL(logger, ConfigManager);

ConfigManager::ConfigManager() {
}

ConfigManager::~ConfigManager() {
}

void ConfigManager::Dump(std::ostream& os) const {
  os << "IOServiceConfig:" << io_service_config_ << std::endl;
}

int ConfigManager::LoadConfig(const std::string& config_path) {
  if (0 != xml_doc_.LoadFile(config_path.c_str())) {
    MI_LOG_ERROR(logger, "ConfigManager::LoadConfig parse xml failed:" << config_path);
    return -1;
  } 

  tinyxml2::XMLElement* root_element = ConfigRoot();

  if (0 != LoadIOServiceConfig(root_element->FirstChildElement("io_service"), &io_service_config_)) {
    return -1;
  }

  if (0 != LoadServicesConfig(root_element->FirstChildElement("services"), &services_config_)) {
    return -1;
  }

  if (0 != LoadClientRoutersConfig(root_element->FirstChildElement("clients"), &client_routers_config_)) {
    return -1;
  }

  if (0 != LoadApplicationConfig(root_element)) {
    return -1;
  }

  return 0;
}

int ConfigManager::LoadIOServiceConfig(tinyxml2::XMLElement* element, IOServiceConfig* config) {
  if (!element) {
    MI_LOG_ERROR(logger, "ConfigManager::LoadIOServiceConfig no element");
    return -1;
  }

  int tmp; 
  XML_LOAD_INT(element, "fd_count", &tmp, -1);
  config->fd_count = tmp;
  XML_LOAD_INT(element, "event_loop_worker", &tmp, -1);
  config->event_loop_worker = tmp;
  XML_LOAD_INT(element, "io_worker", &tmp, -1);
  config->io_worker = tmp;
  XML_LOAD_INT(element, "io_queue_size", &tmp, -1);
  config->io_queue_size = tmp;
  XML_LOAD_INT(element, "service_worker", &tmp, -1);
  config->service_worker = tmp;
  XML_LOAD_INT(element, "service_queue_size", &tmp, -1);
  config->service_queue_size = tmp;
  XML_LOAD_INT(element, "service_timer_worker", &tmp, -1);
  config->service_timer_worker = tmp;
  XML_LOAD_INT(element, "service_timer_queue_size", &tmp, -1);
  config->service_timer_queue_size = tmp;
  XML_LOAD_INT(element, "stack_size", &tmp, -1);
  config->stack_size = tmp;
  XML_LOAD_INT_DEFAULT(element, "matrix_token_bucket", &tmp, 16);
  config->matrix_token_bucket = tmp;
  XML_LOAD_INT_DEFAULT(element, "matrix_queue_bucket", &tmp, 16);
  config->matrix_queue_bucket = tmp;
  XML_LOAD_INT_DEFAULT(element, "matrix_queue_size", &tmp, 1024 * 128);
  config->matrix_queue_size = tmp;

  if (config->io_queue_size & (config->io_queue_size - 1)) {
    MI_LOG_ERROR(logger, "ConfigManager::LoadIOServiceConfig io_queue_size not support:" 
        << config->io_queue_size);
    return -1;
  }

  if (config->service_queue_size & (config->service_queue_size - 1)) {
    MI_LOG_ERROR(logger, "ConfigManager::LoadIOServiceConfig service_queue_size not support:" 
        << config->service_queue_size);
    return -1;
  }

  if (config->service_timer_queue_size & (config->service_timer_queue_size - 1)) {
    MI_LOG_ERROR(logger, "ConfigManager::LoadIOServiceConfig service_timer_queue_size not support:" 
        << config->service_timer_queue_size);
    return -1;   
  }

  if (config->stack_size % 8 != 0) {
    MI_LOG_ERROR(logger, "ConfigManager::LoadIOServiceConfig stack size not support:"
        << config->stack_size);
    return -1;
  }

  return 0;
}

int ConfigManager::LoadServicesConfig(tinyxml2::XMLElement* element, ServicesConfig* config) {
  if (!element) {
    MI_LOG_WARN(logger, "ConfigManager::LoadServicesConfig no element");
    return 0;
  }

  tinyxml2::XMLElement* service = element->FirstChildElement("service");
  while (service) {
    ServiceConfig service_config;

    XML_LOAD_STRING(service, "address", service_config.address, -1);
    XML_LOAD_STRING(service, "name", service_config.name, -1);

    services_config_.push_back(service_config);
    service = service->NextSiblingElement("service");
  }

  return 0;
}

int ConfigManager::LoadClientRoutersConfig(tinyxml2::XMLElement* element, ClientRoutersConfig* config) {
  if (!element) {
    MI_LOG_WARN(logger, "ConfigManager::LoadClientRoutersConfig no element");
    return 0;
  }

  tinyxml2::XMLElement* router = element->FirstChildElement("router");
  while (router) {
    ClientRouterConfig router_config;
    XML_LOAD_STRING(router, "name", router_config.name, -1);
    XML_LOAD_STRING_DEFAULT(router, "router", router_config.router, "round");

    tinyxml2::XMLElement* client = router->FirstChildElement("client");
    while (client) {
      ClientConfig client_config;
      int tmp;
      XML_LOAD_STRING(client, "address", client_config.address, -1);
      XML_LOAD_INT(client, "timeout", &tmp, -1); 
      client_config.timeout = tmp;
      XML_LOAD_INT_DEFAULT(client, "heartbeat", &tmp, 0);
      client_config.heartbeat = tmp;
      XML_LOAD_INT(client, "count", &tmp, -1);
      client_config.count = tmp;

      router_config.clients.push_back(client_config);

      client = client->NextSiblingElement("client");
    }

    config->push_back(router_config);
    router = router->NextSiblingElement("router");
  }

  return 0;
}

int ConfigManager::LoadApplicationConfig(tinyxml2::XMLElement* element) {
  return 0;
}

} //namespace ade 
