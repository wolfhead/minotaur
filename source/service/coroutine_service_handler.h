#ifndef _MINOTAUR_COROUTINE_SERVICE_HANDLER_H_
#define _MINOTAUR_COROUTINE_SERVICE_HANDLER_H_
/**
 * @file coroutine_service_handler.h
 * @author Wolfhead
 */

#include "../event/timer/timer.h"
#include "service_handler.h"

namespace minotaur { 

class CoroutineServiceHandler : public ServiceHandler {
 public:
  typedef ProtocolMessage* MessageType;
  typedef event::Timer<uint64_t> Timer; 

  CoroutineServiceHandler(IOService* io_service);

  virtual void Run(StageData<ServiceHandler>* data);

  virtual ServiceHandler* Clone() {
    return new CoroutineServiceHandler(GetIOService());
  }

 private:
  LOGGER_CLASS_DECL(logger);

  void Process(StageData<ServiceHandler>* data);

  void ProcessTimer();

  void ProcessMessage(StageData<ServiceHandler>* data);

  Timer timer_;
};

} //namespace minotaur


#endif //_MINOTAUR_COROUTINE_SERVICE_HANDLER_H_
