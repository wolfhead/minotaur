#ifndef _MINOTAUR_SERVICE_H_
#define _MINOTAUR_SERVICE_H_
/**
 * @file service.h
 * @author Wolfhead
 */
#include <string>
#include <map>
#include "../common/logger.h"
#include "../matrix/matrix_scope.h"
#include "../matrix/matrix_mem_perf.h"

namespace ade {

class ProtocolMessage;

class Service {
 public:
  Service() : name_("unknown service") {}
  virtual ~Service() {};
  virtual void Serve(ProtocolMessage* message) = 0;

  void SetName(const std::string& name) {name_ = name;}
  inline const std::string& GetName() const {return name_;}
 protected:
  std::string name_;
};

template<typename T>
class GenericService : public Service {
 public:
  explicit GenericService(const T& functor) : functor_(functor) {}

  virtual void Serve(ProtocolMessage* message) {
    matrix::MatrixScope scope(GetName());
    functor_(message);
  }
 private:
  T functor_;
};

template<typename T>
Service* CreateService(const T& functor) {
  return ADE_NEW(GenericService<T>, functor);
}

class ServiceFactory {
 public:
  static ServiceFactory* Instance() {
    static ServiceFactory instance_;
    return &instance_;
  }

  int RegisterService(const std::string& name, Service* service);

  Service* GetService(const std::string& name);

 private:
  LOGGER_CLASS_DECL(logger);

  ServiceFactory();
  ServiceFactory(const ServiceFactory& other);
  ~ServiceFactory();
  ServiceFactory& operator= (const ServiceFactory& other);

  std::map<std::string, Service*> services_;
};

} //namespace ade

#endif // _MINOTAUR_SERVICE_H_
