#ifndef _MINOTAUR_APPLICATION_APPLICATION_H_
#define _MINOTAUR_APPLICATION_APPLICATION_H_
/**
 * @file application.h
 * @auhtor Wolfhead
 */
#include <string>
#include "config_manager.h"
#include "../io_service.h"
#include "../common/logger.h"
#include "../net/acceptor.h"
#include "../net/io_descriptor_factory.h"
#include "../service/service.h"
#include "../service/service_timer_thread.h"

namespace ade {

class ServiceHandler;
class ClientManager;
class ConfigManager;
class Acceptor;

class Application {
 public:
  typedef std::function<void()> TimerFunc;

  template<typename App = Application>
  inline static App* Get() {
    return (App*)application_;
  }

  inline static void Set(Application* app) {
    application_ = app;
  }

  Application();

  virtual ~Application();

  IOService* GetIOService() {return &io_service_;}

  ConfigManager* GetConfigManager() {return config_manager_;}

  ClientManager* GetClientManager() {return client_manager_;}

  int Run(int argc, char* argv[], const std::string& version_data = "");

  int Start(int argc, char* argv[], const std::string& version_data = "");

  int Wait();

  int Stop();

  template<typename T>
  int StartService(const std::string& address, const T& functor) {
    if (0 != RegisterService(address, functor)) {
      return -1;
    }

    Acceptor* acceptor = IODescriptorFactory::Instance()
      .CreateAcceptor(GetIOService(), address, address);
    if (!acceptor) {
      return -1;
    }

    if (0 != acceptor->Start()) {
      IODescriptorFactory::Instance().Destroy(acceptor);
      return -1;
    }

    return 0;
  }

  template<typename T>
  int RegisterService(const std::string& name, const T& functor) {
    return ServiceFactory::Instance()->RegisterService(name, CreateService(functor));
  }

  int RegisterRawService(const std::string& name, Service* service) {
    return ServiceFactory::Instance()->RegisterService(name, service);
  }

  inline uint64_t StartTimer(uint32_t ms, const TimerFunc& func) {
    if (!GetIOService()->GetServiceTimerThread()) {
      return 0;
    }
    return GetIOService()->GetServiceTimerThread()->StartTimer(ms, func);
  }

  inline void CancelTimer(uint64_t timer_id) {
    if (!GetIOService()->GetServiceTimerThread()) {
      return;
    }
    GetIOService()->GetServiceTimerThread()->CancelTimer(timer_id);
  }

  inline int ScheduleTask(const TimerFunc& func) {
    if (!GetIOService()->GetServiceTimerThread()) {
      return -1;
    }
    return GetIOService()->GetServiceTimerThread()->ScheduleTask(func);
  }

  inline uint32_t GetStartTime() const {return start_time_;}

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

  int WritePidFile();

  int StartLogger();

  int LoadConfig();

  int StartIOService();

  int StartClientManager();

  int StartServiceManager();

  int RunIOService();

  int StopClientManager();

  int StopServiceManager();

  int StopService();

  int StopIOService();

  static Application* application_;

  log4cplus::ConfigureAndWatchThread* logger_watcher_;
  IOService io_service_;
  IOServiceConfig io_service_config_;

  ConfigManager* config_manager_;
  ClientManager* client_manager_;

  uint32_t start_time_;

  bool help_mode_;
  bool version_mode_;
  bool daemon_mode_;
  std::string version_info_;
  std::string application_config_;
  std::string logger_config_;

  std::vector<Acceptor*> acceptor_;
};

} //namespace ade

#endif //_MINOTAUR_APPLICATION_APPLICATION_H_
