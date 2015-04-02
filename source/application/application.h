#ifndef _MINOTAUR_APPLICATION_APPLICATION_H_
#define _MINOTAUR_APPLICATION_APPLICATION_H_
/**
 * @file application.h
 * @auhtor Wolfhead
 */
#include <string>
#include "../io_service.h"
#include "../common/logger.h"

namespace minotaur {

class ServiceHandler;
class ConfigManager;

class Application {
 public:
  Application();

  virtual ~Application();

  ConfigManager* GetConfigManager() {return config_manager_;}

  IOService* GetIOService() {return &io_service_;}

  int Run(int argc, char* argv[]);
  
  virtual void DumpStatus(std::ostream& os) const;

 protected:
  virtual int OnStart() = 0;

  virtual int OnStop() = 0;

  virtual int OnFinish() = 0;

  virtual ServiceHandler* CreateServiceHandler() = 0;

  virtual ConfigManager* CreateConfigManager() = 0;

 private:
  LOGGER_CLASS_DECL(logger);

  int ParseCmd(int argc, char* argv[]);

  int InitApplication();

  int StartLogger();

  int LoadConfig();

  int StartIOService();

  int RunIOService();

  int StopIOService();

  log4cplus::ConfigureAndWatchThread* logger_watcher_;
  IOService io_service_;
  IOServiceConfig io_service_config_;

  ConfigManager* config_manager_;

  bool help_mode_;
  bool daemon_mode_;
  std::string version_info_;
  std::string application_config_;
  std::string logger_config_;
};

} //namespace minotaur

#endif //_MINOTAUR_APPLICATION_APPLICATION_H_
