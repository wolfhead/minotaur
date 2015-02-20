#ifndef _MINOTAUR_NET_IO_HANDLER_H_
#define _MINOTAUR_NET_IO_HANDLER_H_

#include "../common/logger.h"
#include "../stage.h"
#include "io_message.h"

namespace minotaur {

class IOService;
class IOHandler;

class IOHandlerFactory {
 public:
  typedef IOHandler Handler;
  typedef Stage<IOHandlerFactory> StageType;

  IOHandlerFactory(IOService* io_service);

  IOHandler* Create(StageType* stage);
 private:
  IOService* io_service_;
};


class IOHandler {
 public:
  typedef IOHandler self;
  typedef Stage<IOHandlerFactory> StageType;
  typedef IOMessage MessageType;

  static const bool share_handler = false;
  static const bool share_queue = false;

  static uint32_t HashMessage(const IOMessage& message, uint32_t worker_count);

  IOHandler(IOService* service, StageType* stage);

  void SetStage(StageType* stage) {stage_ = stage;}

  void Handle(const IOMessage& message);

  IOService* GetIOService() {return io_service_;}

 private:
  LOGGER_CLASS_DECL(logger);

  void HandleIOReadEvent(const IOMessage& message);

  void HandleIOWriteEvent(const IOMessage& message);

  void HandleIOCloseEvent(const IOMessage& message);

  void HandleIOMessageEvent(const IOMessage& message);

  IOService* io_service_;
  StageType* stage_;
};


} // namespace minotaur

#endif // _MINOTAUR_NET_IO_HANDLER_H_
