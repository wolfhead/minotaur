/**
 * @file io_handler.cpp
 * @author Wolfhead
 */
#include "io_handler.h"

namespace minotaur {

LOGGER_CLASS_IMPL_NAME(logger, IOHandler, "net.io_handler");

IOHandler::IOHandler(IOService* io_service, StageType* stage)
    : io_service_(io_service)
    , stage_(stage) {
}

void IOHandler::Handle(IOMessageBase* message) {
  switch (message->type_id) {
    case minotaur::MessageType::kReadEvent:
      return HandleReadEvent(message);
    case minotaur::MessageType::kWriteEvent:
      return HandleWriteEvent(message);
    default:
      MI_LOG_ERROR(logger, "IOHanlder unknown message:" << message->type_id);
      MessageFactory::Destory(message);
      return;
  }
}

void IOHandler::HandleReadEvent(IOMessageBase* message) {
  MI_LOG_TRACE(logger, "HandleReadEvent");
}

void IOHandler::HandleWriteEvent(IOMessageBase* message) {
  MI_LOG_TRACE(logger, "HandleWriteEvent");
}

IOHandlerFactory::IOHandlerFactory(IOService* io_service) 
    : io_service_(io_service) {
}

IOHandler* IOHandlerFactory::Create(StageType* stage) {
  return new IOHandler(io_service_, stage);
}


} //namespace minotaur
