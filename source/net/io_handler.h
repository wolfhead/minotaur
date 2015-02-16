#ifndef _MINOTAUR_NET_IO_HANDLER_H_
#define _MINOTAUR_NET_IO_HANDLER_H_

#include "../common/logger.h"
#include "../stage.h"
#include "../message.h"

namespace minotaur {

class IOHanlder {
 public:
  typedef IOHanlder self;
  typedef minotaur::Stage<self> StageType;
  typedef MessageBase* MessageType;

  static const bool share_handler = false;
  static const bool share_queue = false;

  static uint32_t HashnMessage(const MessageBase* message, uint32_t worker_count) {
    return message->channel_id() % worker_count; 
  }

  void SetStage(StageType* stage) {stage_ = stage;}

 private:
  LOGGER_CLASS_DECL(logger);

  void Handle(MessageBase* message);

  void HandleReadEvent(MessageBase* message);

  void HandleWriteEvent(MessageBase* message);

  StageType* stage_;
};

} // namespace minotaur

#endif // _MINOTAUR_NET_IO_HANDLER_H_
