/**
 * @file io_handler.cpp
 * @author Wolfhead
 */
#include "io_handler.h"

namespace minotaur {

LOGGER_CLASS_IMPL_NAME(logger, IOHanlder, "net.io_handler");

void IOHanlder::Handle(MessageBase* message) {
  switch (message->type_id()) {
    case minotaur::MessageType::kReadEvent:
      return HandleReadEvent(message);
    case minotaur::MessageType::kWriteEvent:
      return HandleWriteEvent(message);
    default:
      MI_LOG_ERROR(logger, "IOHanlder unknown message:" << message->type_id());
      MessageFactory::Destory(message);
      return;
  }
}

void IOHanlder::HandleReadEvent(MessageBase* message) {
}

void IOHanlder::HandleWriteEvent(MessageBase* message) {
}

} //namespace minotaur
