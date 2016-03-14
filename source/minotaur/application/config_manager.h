#ifndef _MINOTAUR_CONFIG_MANAGER_H_
#define _MINOTAUR_CONFIG_MANAGER_H_
/**
 * @file config_manager.h
 * @author Wolfhead
 */
#include "../io_service.h"
#include "../3rd-party/tinyxml2/tinyxml2.h"
#include "../client/client_manager.h"

namespace ade {

struct ServiceConfig {
  std::string address;
  std::string name;
};

class ConfigManager {
 public:
  typedef std::vector<ServiceConfig> ServicesConfig;
  typedef std::vector<ClientRouterConfig> ClientRoutersConfig;

  ConfigManager();
  virtual ~ConfigManager();

  int LoadConfig(const std::string& config_path);

  tinyxml2::XMLElement* ConfigRoot() {return xml_doc_.RootElement();}

  IOServiceConfig& GetIOServiceConfig() {return io_service_config_;}

  ServicesConfig GetServicesConfig() {return services_config_;}

  ClientRoutersConfig GetClientRoutersConfig() {return client_routers_config_;}

  virtual void Dump(std::ostream& os) const;

 protected:
  virtual int LoadIOServiceConfig(tinyxml2::XMLElement* element, IOServiceConfig* config);

  virtual int LoadApplicationConfig(tinyxml2::XMLElement* element);

  virtual int LoadServicesConfig(tinyxml2::XMLElement*, ServicesConfig* config);

  virtual int LoadClientRoutersConfig(tinyxml2::XMLElement*, ClientRoutersConfig* config);

 private:
  LOGGER_CLASS_DECL(logger);

  tinyxml2::XMLDocument xml_doc_;

  IOServiceConfig io_service_config_;

  ServicesConfig services_config_;

  ClientRoutersConfig client_routers_config_;
};

} //namespace ade

#endif //_MINOTAUR_CONFIG_MANAGER_H_
