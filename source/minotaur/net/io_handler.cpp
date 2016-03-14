/**
 * @file io_handler.cpp
 * @author Wolfhead
 */
#include "io_handler.h"
#include <sys/prctl.h>
#include "io_descriptor_factory.h"
#include "io_descriptor.h"
#include "channel.h"
#include "../io_service.h"
#include "../service/service_handler.h"
#include "../matrix/matrix_scope.h"

namespace ade {

LOGGER_CLASS_IMPL_NAME(logger, IOHandler, "net.io_handler");
thread_local IOHandler* IOHandler::current_io_handler_ = NULL;


IOHandler::IOHandler(IOService* io_service)
    : timer_(5, 1000 * 60 * 60){
  SetIOService(io_service);
}

void IOHandler::ProcessTimer() {
  Timer::NodeType* timer_head = timer_.ProcessTimer();
  Timer::NodeType* current = timer_head;
  while (current) {
    if (current->active) {
      current->data();
    }
    current = current->next;
  }
  if (timer_head) {
    timer_.DestroyTimerNode(timer_head);
  }
}

void IOHandler::Run(StageData<IOHandler>* data) {
  prctl(PR_SET_NAME, "io_handler");
  current_io_handler_ = this;

  EventMessage* message;
  while (data->running) {
    ProcessTimer();
    if (!data->queue->Pop(&message, 1)) {
      continue;
    }
    Handle(message);    
  }
}

void IOHandler::Handle(EventMessage* message) {
  MI_LOG_TRACE(logger, "IOHandler::Handle " << *message);

  switch (message->type_id) {
    case ade::MessageType::kIOEvent:
      {
        if (message->payload & event::EventType::EV_CLOSE) {
          HandleIOCloseEvent(message);
          break;
        }
        if (message->payload & event::EventType::EV_READ) {
          HandleIOReadEvent(message);
        }
        if (message->payload & event::EventType::EV_WRITE) {
          HandleIOWriteEvent(message);
        }
        break;
      }
    case ade::MessageType::kIOMessageEvent:
      HandleIOMessageEvent(message);
      break;
    case ade::MessageType::kIOActiveCloseEvent:
      HandleIOActiveCloseEvent(message);
      break;
    default:
      MI_LOG_ERROR(logger, "IOHanlder unknown message:" << message->type_id);
      break;
  }

  MessageFactory::Destroy(message);
}

void IOHandler::HandleIOReadEvent(EventMessage* message) {
  IODescriptor* descriptor = 
      IODescriptorFactory::GetIODescriptor(message->descriptor_id);
  if (!descriptor) {
    MI_LOG_DEBUG(logger, "HandleReadEvent descriptor not found:"
        << message->descriptor_id);
    return;
  }
  
  MI_LOG_TRACE(logger, "HandleIOReadEvent:" << *descriptor);

  descriptor->OnRead();
}

void IOHandler::HandleIOWriteEvent(EventMessage* message) {
  IODescriptor* descriptor = 
      IODescriptorFactory::GetIODescriptor(message->descriptor_id);
  if (!descriptor) {
    MI_LOG_DEBUG(logger, "HandleWriteEvent descriptor not found:"
        << message->descriptor_id);
    return;
  }

  MI_LOG_TRACE(logger, "HandleIOWriteEvent:" << *descriptor);

  descriptor->OnWrite();
}

void IOHandler::HandleIOCloseEvent(EventMessage* message) {
  MI_LOG_TRACE(logger, "HandleIOCloseEvent");
  IODescriptor* descriptor = 
      IODescriptorFactory::GetIODescriptor(message->descriptor_id);
  if (!descriptor) {
    MI_LOG_DEBUG(logger, "HandleIOCloseEvent descriptor not found:"
        << message->descriptor_id);
    return;
  }

  descriptor->OnClose();
}

void IOHandler::HandleIOMessageEvent(EventMessage* message) {
  MI_LOG_TRACE(logger, "HandleIOMessageEvent");
  IODescriptor* descriptor = 
      IODescriptorFactory::GetIODescriptor(message->descriptor_id);
  if (!descriptor) {
    MI_LOG_DEBUG(logger, "HandleIOCloseEvent descriptor not found:"
        << message->descriptor_id);
    HandleIOMessageFailure(message);
    return;
  }

  Channel* channel = dynamic_cast<Channel*>(descriptor);
  if (!channel) {
    MI_LOG_ERROR(logger, "IOHandler::HandleIOMessageEvent descriptor is not channel");
    HandleIOMessageFailure(message);
    return;
  }

  ProtocolMessage* protocol_message = message->GetProtocolMessage();
  if (0 != channel->EncodeMessage(protocol_message)) {
    HandleIOMessageFailure(message);
    return;    
  }

  descriptor->OnWrite();
}

void IOHandler::HandleIOActiveCloseEvent(EventMessage* message) {
  MI_LOG_TRACE(logger, "HandleIOActiveCloseEvent");
  IODescriptor* descriptor = 
      IODescriptorFactory::GetIODescriptor(message->descriptor_id);
  if (!descriptor) {
    MI_LOG_DEBUG(logger, "HandleIOActiveCloseEvent descriptor not found:"
        << message->descriptor_id);
    return;
  }

  descriptor->OnActiveClose();
}

void IOHandler::HandleIOMessageFailure(EventMessage* message) {
  ProtocolMessage* protocol_message = message->GetProtocolMessage();
  if (protocol_message->direction == ProtocolMessage::kOutgoingRequest) {
    protocol_message->status = ProtocolMessage::kInternalFailure;
    if (!GetIOService()->GetServiceStage()->Send(protocol_message)) {
      MI_LOG_WARN(logger, "IOHandler::HandleIOMessageFailure send fail");
    }
  } else {
    message->DestroyPayload();
  }
}

IOHandler* IOHandler::Clone() {
  return new IOHandler(GetIOService());
}


} //namespace ade
