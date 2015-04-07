#ifndef _MINOTAUR_CONFIG_MANAGER_H_
#define _MINOTAUR_CONFIG_MANAGER_H_
/**
 * @file config_manager.h
 * @author Wolfhead
 */
#include "../io_service.h"
#include "../3rd-party/tinyxml2/tinyxml2.h"

namespace minotaur {

struct ServiceConfig {
  std::string address;
  std::string name;
};

class ConfigManager {
 public:
  typedef std::vector<ServiceConfig> ServicesConfig;

  ConfigManager();
  virtual ~ConfigManager();

  int LoadConfig(const std::string& config_path);

  IOServiceConfig& GetIOServiceConfig() {return io_service_config_;}

  ServicesConfig GetServicesConfig() {return services_config_;}

 protected:
  virtual int LoadIOServiceConfig(tinyxml2::XMLElement* element, IOServiceConfig* config);

  virtual int LoadApplicationConfig(tinyxml2::XMLElement* element);

  virtual int LoadServicesConfig(tinyxml2::XMLElement*, ServicesConfig* config);

 private:
  LOGGER_CLASS_DECL(logger);

  IOServiceConfig io_service_config_;

  ServicesConfig services_config_;
};

} //namespace minotaur

#endif //_MINOTAUR_CONFIG_MANAGER_H_
