/**
 * @file service_handler.cpp
 * @author Wolfhead
 */
#include "service_handler.h"
#include <sys/prctl.h>
#include "../net/io_descriptor.h"
#include "../net/io_descriptor_factory.h"

namespace ade {

LOGGER_CLASS_IMPL_NAME(logger, ServiceHandler, "ServiceHandlerBase");

ServiceHandler::ServiceHandler(IOService* io_service) {
  SetIOService(io_service);
}

void ServiceHandler::Run(StageData<ServiceHandler>* data) {
  prctl(PR_SET_NAME, "service_handler");

  ProtocolMessage* message;
  while (data->running) {
    if (!data->queue->Pop(&message, 2)) {
      continue;
    }
    Handle(message);    
  }
}

void ServiceHandler::Handle(ProtocolMessage* protocol_message) {
  switch (protocol_message->type_id) {
    case ade::MessageType::kLineMessage:
      return protocol_message->direction == ProtocolMessage::kIncomingRequest 
          ? OnLineRequestMessage(static_cast<LineMessage*>(protocol_message))
          : OnLineResponseMessage(static_cast<LineMessage*>(protocol_message));
    case ade::MessageType::kHttpMessage:
      return protocol_message->direction == ProtocolMessage::kIncomingRequest 
          ? OnHttpRequestMessage(static_cast<HttpMessage*>(protocol_message))
          : OnHttpResponseMessage(static_cast<HttpMessage*>(protocol_message));
    case ade::MessageType::kRapidMessage:
      return protocol_message->direction == ProtocolMessage::kIncomingRequest 
          ? OnRapidRequestMessage(static_cast<RapidMessage*>(protocol_message))
          : OnRapidResponseMessage(static_cast<RapidMessage*>(protocol_message));
    default:
      return OnUnknownProtocolMessage(protocol_message);
  }
}

void ServiceHandler::OnLineRequestMessage(
    LineMessage* message) {
  MI_LOG_WARN(logger, "not implement:" << message->type_id);
  MessageFactory::Destroy(message);
}

void ServiceHandler::OnLineResponseMessage(
    LineMessage* message) {
  MI_LOG_WARN(logger, "not implement:" << message->type_id);
  MessageFactory::Destroy(message);
}

void ServiceHandler::OnHttpRequestMessage(
    HttpMessage* message) {
  MI_LOG_WARN(logger, "not implement:" << message->type_id);
  MessageFactory::Destroy(message);
}

void ServiceHandler::OnHttpResponseMessage(
    HttpMessage* message) {
  MI_LOG_WARN(logger, "not implement:" << message->type_id);
  MessageFactory::Destroy(message);
}

void ServiceHandler::OnRapidRequestMessage(
    RapidMessage* message) {
  MI_LOG_WARN(logger, "not implement:" << message->type_id);
  MessageFactory::Destroy(message);
}

void ServiceHandler::OnRapidResponseMessage(
    RapidMessage* message) {
  MI_LOG_WARN(logger, "not implement:" << message->type_id);
  MessageFactory::Destroy(message);
}

void ServiceHandler::OnUnknownProtocolMessage(
    ProtocolMessage* message) {
  MI_LOG_WARN(logger, "unknown protocol:" << *message);
  MessageFactory::Destroy(message);
}

ServiceHandler* ServiceHandler::Clone() {
  return new ServiceHandler(GetIOService()); 
}

} //namespace ade
