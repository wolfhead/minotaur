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
    case minotaur::MessageType::kLineMessage:
      return protocol_message->direction == ProtocolMessage::kIncomingRequest 
          ? OnLineRequestMessage(static_cast<LineMessage*>(protocol_message))
          : OnLineResponseMessage(static_cast<LineMessage*>(protocol_message));
    case minotaur::MessageType::kHttpMessage:
      return protocol_message->direction == ProtocolMessage::kIncomingRequest 
          ? OnHttpRequestMessage(static_cast<HttpMessage*>(protocol_message))
          : OnHttpResponseMessage(static_cast<HttpMessage*>(protocol_message));
    case minotaur::MessageType::kRapidMessage:
      return protocol_message->direction == ProtocolMessage::kIncomingRequest 
          ? OnRapidRequestMessage(static_cast<RapidMessage*>(protocol_message))
          : OnRapidResponseMessage(static_cast<RapidMessage*>(protocol_message));
    default:
      return OnUnknownProtocolMessage(protocol_message);
  }
}

void ServiceHandlerBase::OnLineRequestMessage(
    LineMessage* message) {
  MI_LOG_WARN(logger, "not implement:" << message->type_id);
  MessageFactory::Destroy(message);
}

void ServiceHandlerBase::OnLineResponseMessage(
    LineMessage* message) {
  MI_LOG_WARN(logger, "not implement:" << message->type_id);
  MessageFactory::Destroy(message);
}

void ServiceHandlerBase::OnHttpRequestMessage(
    HttpMessage* message) {
  MI_LOG_WARN(logger, "not implement:" << message->type_id);
  MessageFactory::Destroy(message);
}

void ServiceHandlerBase::OnHttpResponseMessage(
    HttpMessage* message) {
  MI_LOG_WARN(logger, "not implement:" << message->type_id);
  MessageFactory::Destroy(message);
}

void ServiceHandlerBase::OnRapidRequestMessage(
    RapidMessage* message) {
  MI_LOG_WARN(logger, "not implement:" << message->type_id);
  MessageFactory::Destroy(message);
}

void ServiceHandlerBase::OnRapidResponseMessage(
    RapidMessage* message) {
  MI_LOG_WARN(logger, "not implement:" << message->type_id);
  MessageFactory::Destroy(message);
}

void ServiceHandlerBase::OnUnknownProtocolMessage(
    ProtocolMessage* message) {
  MI_LOG_WARN(logger, "unknown protocol:" << *message);
  MessageFactory::Destroy(message);
}

} //namespace minotaur
