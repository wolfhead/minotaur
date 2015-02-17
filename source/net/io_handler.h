#ifndef _MINOTAUR_NET_IO_HANDLER_H_
#define _MINOTAUR_NET_IO_HANDLER_H_

#include "../common/logger.h"
#include "../stage.h"
#include "../message.h"

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
  typedef IOMessageBase* MessageType;

  static const bool share_handler = false;
  static const bool share_queue = false;

  static uint32_t HashMessage(const IOMessageBase* message, uint32_t worker_count) {
    return message->channel_id % worker_count; 
  }

  IOHandler(IOService* service, StageType* stage);

  void SetStage(StageType* stage) {stage_ = stage;}

  void Handle(IOMessageBase* message);

 private:
  LOGGER_CLASS_DECL(logger);

  void HandleReadEvent(IOMessageBase* message);

  void HandleWriteEvent(IOMessageBase* message);

  IOService* io_service_;
  StageType* stage_;
};


} // namespace minotaur

#endif // _MINOTAUR_NET_IO_HANDLER_H_
