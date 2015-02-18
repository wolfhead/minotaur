/**
 * @file io_handler.cpp
 * @author Wolfhead
 */
#include "io_handler.h"
#include "../io_service.h"
#include "io_descriptor_factory.h"
#include "io_descriptor.h"

namespace minotaur {

LOGGER_CLASS_IMPL_NAME(logger, IOHandler, "net.io_handler");

IOHandler::IOHandler(IOService* io_service, StageType* stage)
    : io_service_(io_service)
    , stage_(stage) {
}

void IOHandler::Handle(const IOMessage& message) {
  switch (message.type_id) {
    case minotaur::MessageType::kIOReadEvent:
      return HandleIOReadEvent(message);
    case minotaur::MessageType::kIOWriteEvent:
      return HandleIOWriteEvent(message);
    case minotaur::MessageType::kIOCloseEvent:
      return HandleIOCloseEvent(message);
    default:
      MI_LOG_ERROR(logger, "IOHanlder unknown message:" << message.type_id);
      return;
  }
}

void IOHandler::HandleIOReadEvent(const IOMessage& message) {
  IODescriptor* descriptor = GetIOService()->GetIODescriptorFactory()
    ->GetIODescriptor(message.descriptor_id);
  if (!descriptor) {
    MI_LOG_DEBUG(logger, "HandleReadEvent descriptor not found:"
        << message.descriptor_id);
    return;
  }

  descriptor->OnRead();
}

void IOHandler::HandleIOWriteEvent(const IOMessage& message) {
  MI_LOG_TRACE(logger, "HandleWriteEvent");
  IODescriptor* descriptor = GetIOService()->GetIODescriptorFactory()
    ->GetIODescriptor(message.descriptor_id);
  if (!descriptor) {
    MI_LOG_DEBUG(logger, "HandleWriteEvent descriptor not found:"
        << message.descriptor_id);
    return;
  }

  descriptor->OnWrite();
}

void IOHandler::HandleIOCloseEvent(const IOMessage& message) {
  MI_LOG_TRACE(logger, "HandleWriteEvent");
  IODescriptor* descriptor = GetIOService()->GetIODescriptorFactory()
    ->GetIODescriptor(message.descriptor_id);
  if (!descriptor) {
    MI_LOG_DEBUG(logger, "HandleIOCloseEvent descriptor not found:"
        << message.descriptor_id);
    return;
  }

  descriptor->OnClose();
}

IOHandlerFactory::IOHandlerFactory(IOService* io_service) 
    : io_service_(io_service) {
}

IOHandler* IOHandlerFactory::Create(StageType* stage) {
  return new IOHandler(io_service_, stage);
}


} //namespace minotaur
