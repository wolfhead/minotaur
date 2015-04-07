#ifndef _MINOTAUR_SERVICE_H_
#define _MINOTAUR_SERVICE_H_
/**
 * @file service.h
 * @author Wolfhead
 */
#include <string>
#include <map>
#include "../common/logger.h"

namespace minotaur {

class ProtocolMessage;

class Service {
 public:
  virtual ~Service() {};
  virtual void Serve(ProtocolMessage* message) = 0;
};

template<typename T>
class GenericService : public Service {
 public:
  GenericService(const T& functor) : functor_(functor) {}

  virtual void Serve(ProtocolMessage* message) {
    functor_(message);
  }
 private:
  T functor_;
};

template<typename T>
Service* CreateService(const T& functor) {
  return new GenericService<T>(functor);
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

} //namespace minotaur

#endif // _MINOTAUR_SERVICE_H_
