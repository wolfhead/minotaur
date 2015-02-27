#ifndef _MINOTAUR_SERVICE_HANDLER_H_
#define _MINOTAUR_SERVICE_HANDLER_H_
/**
 * @file service_handler.h
 * @author Wolfhead
 */
#include "../io_service.h"
#include "../message.h"
#include "../common/logger.h"
#include "../net/io_message.h"

namespace minotaur {

class ServiceHandlerBase;

// this is a dummy class
class ServiceHandlerFactory {
 public:
  typedef ServiceHandlerBase Handler;
  typedef Stage<ServiceHandlerFactory> StageType;
  
  virtual ~ServiceHandlerFactory() {}

  virtual ServiceHandlerBase* Create(StageType* stage) = 0;

  ServiceHandlerFactory* BindIOService(IOService* io_service) {
    io_service_ = io_service;
    return this;
  } 

 protected:
  IOService* io_service_;
};

template<typename ServiceType>
class GenericServiceHandlerFactory : public ServiceHandlerFactory{
 public:
  typedef ServiceType Handler;
  typedef Stage<ServiceHandlerFactory> StageType;
   
  ServiceType* Create(StageType* stage) {
    return new ServiceType(io_service_, stage);
  }
};

class ServiceHandlerBase {
 public:
  typedef Stage<ServiceHandlerFactory> StageType;
  typedef EventMessage MessageType;

  static const bool share_handler = true;
  static const bool share_queue = true;

  ServiceHandlerBase(IOService* io_service, StageType* stage);
  virtual ~ServiceHandlerBase();

  virtual int Start();

  virtual int Stop();

  static uint32_t HashMessage(const EventMessage& message, uint32_t worker_count);

  virtual void Handle(const EventMessage& message);

  IOService* GetIOService() {return io_service_;}

 protected:

  virtual void OnUnknownEvent(const EventMessage& message);

  virtual void OnIOMessageEvent(const EventMessage& message);

  virtual void OnLineProtocolMessage(LineProtocolMessage* message);

  virtual void OnHttpProtocolMessage(HttpProtocolMessage* message);

  virtual void OnUnknownProtocolMessage(ProtocolMessage* message);

  IOService* io_service_;
  StageType* stage_;
 private:
  LOGGER_CLASS_DECL(logger);
};

} //namespace minotaur

#endif //_MINOTAUR_SERVICE_HANDLER_H_
