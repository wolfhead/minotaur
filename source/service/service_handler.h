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
#include "../handler.h"

namespace minotaur {

class ServiceHandler : public HandlerSkeleton {
 public:

  ServiceHandler(IOService* io_service);

  void Run(StageData* data);

  void Handle(const EventMessage& message);

  virtual ServiceHandler* Clone();

  uint32_t Hash(const EventMessage& message) {
    static std::atomic<uint32_t> round_robin;
    uint16_t handler_id = message.GetProtocolMessage()->handler_id;
    if (handler_id != Handler::kUnspecifiedId) {
      return handler_id;
    }
    return ++round_robin;
  }

 protected:

  virtual void OnUnknownEvent(const EventMessage& message);

  virtual void OnIOMessageEvent(const EventMessage& message);

  virtual void OnLineRequestMessage(LineMessage* message);

  virtual void OnLineResponseMessage(LineMessage* message);

  virtual void OnHttpRequestMessage(HttpMessage* message);

  virtual void OnHttpResponseMessage(HttpMessage* message);

  virtual void OnRapidRequestMessage(RapidMessage* message);

  virtual void OnRapidResponseMessage(RapidMessage* message);

  virtual void OnUnknownProtocolMessage(ProtocolMessage* message);

 private:
  LOGGER_CLASS_DECL(logger);
};

} //namespace minotaur

#endif //_MINOTAUR_SERVICE_HANDLER_H_
