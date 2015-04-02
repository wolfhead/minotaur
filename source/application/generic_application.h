#ifndef MINOTAUR_GENERIC_APPLICATION_H
#define MINOTAUR_GENERIC_APPLICATION_H
/**
 * @file generic_application.h
 * @author Wolfhead
 */
#include <functional>
#include "application.h"
#include "../net/acceptor.h"
#include "../net/io_descriptor_factory.h"
#include "../service/service.h"

namespace minotaur {

template<typename ConfigManagerType, typename ServiceHandlerType>
class GenericApplication : public Application {
 public:
  typedef std::function<int()> Func;

  GenericApplication& SetOnStart(const Func& func) {
    on_start_func_ = func;
    return *this;
  }

  GenericApplication& SetOnStop(const Func& func) {
    on_stop_func_ = func;
    return *this;
  } 
  
  GenericApplication& SetOnFinish(const Func& func) {
    on_finish_func_ = func;
    return *this;
  }

  template<typename T>
  int StartService(const std::string& address, const T& functor) {
    if (0 != ServiceManager::Instance()->RegisterService(address, CreateService(functor))) {
      return -1;
    }

    Acceptor* acceptor = IODescriptorFactory::Instance()
      .CreateAcceptor(GetIOService(), address, address);
    if (0 != acceptor->Start()) {
      IODescriptorFactory::Instance().Destroy(acceptor);
      return -1;
    }

    return 0;
  }

 protected:
  virtual ServiceHandler* CreateServiceHandler() {
    return new ServiceHandlerType(GetIOService());
  }

  virtual ConfigManager* CreateConfigManager() {
    return new ConfigManagerType();
  }

  virtual int OnStart() {
    if (on_start_func_) {
      return on_start_func_();
    }
    return 0;
  } 

  virtual int OnStop() {
    if (on_stop_func_) {
      return on_stop_func_();
    }
    return 0;
  }

  virtual int OnFinish() {
    if (on_finish_func_) {
      return on_finish_func_();
    }
    return 0;
  }

 private:
  Func on_start_func_;
  Func on_stop_func_;
  Func on_finish_func_;
};

} //namespace minotaur

#endif //MINOTAUR_GENERIC_APPLICATION_H
