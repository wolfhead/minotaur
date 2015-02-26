/**
 * @file service_handler.cpp
 * @author Wolfhead
 */
#include "service_handler.h"
#include "../net/io_descriptor.h"
#include "../net/io_descriptor_factory.h"

namespace minotaur {

LOGGER_CLASS_IMPL_NAME(logger, ServiceHandlerBase, "ServiceHandlerBase");

uint32_t ServiceHandlerBase::HashMessage(
    const EventMessage& message, 
    uint32_t worker_count) {
  IODescriptor* descriptor = 
      IODescriptorFactory::GetIODescriptor(message.descriptor_id);
  if (descriptor) {
    return descriptor->GetIN() % worker_count;
  } else {
    MI_LOG_ERROR(logger, "ServiceHandlerBase::HashMessage descriptor not found:"
        << message);
    return (message.descriptor_id) % worker_count; 
  }
}

ServiceHandlerBase::ServiceHandlerBase(
    IOService* io_service, 
    StageType* stage)
    : io_service_(io_service)
    , stage_(stage) {
}

ServiceHandlerBase::~ServiceHandlerBase() {
}

int ServiceHandlerBase::Start() {
  MI_LOG_INFO(logger, "ServiceHandlerBase::Start");
  return 0;
}

int ServiceHandlerBase::Stop() {
  MI_LOG_INFO(logger, "ServiceHandlerBase::Stop");
  return 0;
}

void ServiceHandlerBase::Handle(const EventMessage& message) {
  switch (message.type_id) {
    case minotaur::MessageType::kIOMessageEvent:
      return OnIOMessageEvent(message);
    default:
      return OnUnknownEvent(message);
  }
}

void ServiceHandlerBase::OnUnknownEvent(const EventMessage& message) {
  MI_LOG_WARN(logger, "unknown event:" << message.type_id);
  message.Destroy();
}

void ServiceHandlerBase::OnIOMessageEvent(const EventMessage& message) {
  ProtocolMessage* protocol_message = message.GetProtocolMessage();
  switch (protocol_message->type_id) {
    case minotaur::MessageType::kLineProtocolMessage:
      return OnLineProtocolMessage(
          static_cast<LineProtocolMessage*>(protocol_message));
    case minotaur::MessageType::kHttpProtocolMessage:
      return OnHttpProtocolMessage(
          static_cast<HttpProtocolMessage*>(protocol_message));
    default:
      return OnUnknownProtocolMessage(protocol_message);
  }
}

void ServiceHandlerBase::OnLineProtocolMessage(
    LineProtocolMessage* message) {
  MI_LOG_WARN(logger, "not implement:" << message->type_id);
  MessageFactory::Destroy(message);
}

void ServiceHandlerBase::OnHttpProtocolMessage(
    HttpProtocolMessage* message) {
  MI_LOG_WARN(logger, "not implement:" << message->type_id);
  MessageFactory::Destroy(message);
}

void ServiceHandlerBase::OnUnknownProtocolMessage(
    ProtocolMessage* message) {
  MI_LOG_WARN(logger, "unknown protocol:" << *message);
  MessageFactory::Destroy(message);
}

} //namespace minotaur
