/**
 * @file config_manager.cpp
 * @author Wolfhead
 */
#include "config_manager.h"
#include "../common/xml_macro.h"

namespace minotaur {

LOGGER_CLASS_IMPL(logger, ConfigManager);

ConfigManager::ConfigManager() {
}

ConfigManager::~ConfigManager() {
}

int ConfigManager::LoadConfig(const std::string& config_path) {
  tinyxml2::XMLDocument xml_doc;
  if (0 != xml_doc.LoadFile(config_path.c_str())) {
    MI_LOG_ERROR(logger, "ConfigManager::LoadConfig parse xml failed:" << config_path);
    return -1;
  } 

  tinyxml2::XMLElement* root_element = xml_doc.RootElement();

  if (0 != LoadIOServiceConfig(root_element->FirstChildElement("io_service"), &io_service_config_)) {
    return -1;
  }

  if (0 != LoadServicesConfig(root_element->FirstChildElement("services"), &services_config_)) {
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
  XML_LOAD_INT(element, "stack_size", &tmp, -1);
  config->stack_size = tmp;

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

  if (config->stack_size % 8 != 0) {
    MI_LOG_ERROR(logger, "ConfigManager::LoadIOServiceConfig stack size not support:"
        << config->stack_size);
    return -1;
  }

  return 0;
}

int ConfigManager::LoadServicesConfig(tinyxml2::XMLElement* element, ServicesConfig* config) {
  if (!element) {
    MI_LOG_ERROR(logger, "ConfigManager::LoadServicesConfig no element");
    return -1;
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

int ConfigManager::LoadApplicationConfig(tinyxml2::XMLElement* element) {
  return 0;
}

} //namespace minotaur 
