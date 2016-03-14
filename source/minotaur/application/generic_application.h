#ifndef MINOTAUR_GENERIC_APPLICATION_H
#define MINOTAUR_GENERIC_APPLICATION_H
/**
 * @file generic_application.h
 * @author Wolfhead
 */
#include <functional>
#include "application.h"

namespace ade {

template<typename ConfigManagerType, typename ServiceHandlerType>
class GenericApplication : public Application {
 public:
  typedef std::function<int()> DelegateFunc;

  GenericApplication& SetOnStart(const DelegateFunc& func) {
    on_start_func_ = func;
    return *this;
  }

  GenericApplication& SetOnStop(const DelegateFunc& func) {
    on_stop_func_ = func;
    return *this;
  } 
  
  GenericApplication& SetOnFinish(const DelegateFunc& func) {
    on_finish_func_ = func;
    return *this;
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
  DelegateFunc on_start_func_;
  DelegateFunc on_stop_func_;
  DelegateFunc on_finish_func_;
};

} //namespace ade

#endif //MINOTAUR_GENERIC_APPLICATION_H
