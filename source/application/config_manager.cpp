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

  if (0 != LoadIOServiceConfig(xml_doc.RootElement(), &io_service_config_)) {
    return -1;
  }

  if (0 != LoadApplicationConfig(xml_doc.RootElement())) {
    return -1;
  }

  return 0;
}

int ConfigManager::LoadIOServiceConfig(tinyxml2::XMLElement* element, IOServiceConfig* config) {
  if (!element) {
    MI_LOG_ERROR(logger, "ConfigManager::LoadIOServiceConfig no element");
    return -1;
  }

  tinyxml2::XMLElement* config_element = element->FirstChildElement("io_service");
  if (!config_element) {
    MI_LOG_ERROR(logger, "ConfigManager::LoadIOServiceConfig no io_service element");
    return -1;
  }

  int tmp; 
  XML_LOAD_INT(config_element, "fd_count", &tmp, -1);
  config->fd_count = tmp;
  XML_LOAD_INT(config_element, "event_loop_worker", &tmp, -1);
  config->event_loop_worker = tmp;
  XML_LOAD_INT(config_element, "io_worker", &tmp, -1);
  config->io_worker = tmp;
  XML_LOAD_INT(config_element, "io_queue_size", &tmp, -1);
  config->io_queue_size = tmp;
  XML_LOAD_INT(config_element, "service_worker", &tmp, -1);
  config->service_worker = tmp;
  XML_LOAD_INT(config_element, "service_queue_size", &tmp, -1);
  config->service_queue_size = tmp;

  if (config->io_queue_size & (config->io_queue_size - 1)) {
    MI_LOG_ERROR(logger, "Configmanager::LoadIOServiceConfig io_queue_size not support:" 
        << config->io_queue_size);
    return -1;
  }

  if (config->service_queue_size & (config->service_queue_size - 1)) {
    MI_LOG_ERROR(logger, "Configmanager::LoadIOServiceConfig service_queue_size not support:" 
        << config->service_queue_size);
    return -1;
  }

  return 0;
}

int ConfigManager::LoadApplicationConfig(tinyxml2::XMLElement* element) {
  return 0;
}

} //namespace minotaur 
