/**
 * @file io_handler.cpp
 * @author Wolfhead
 */
#include "io_handler.h"
#include <sys/prctl.h>
#include "../io_service.h"
#include "io_descriptor_factory.h"
#include "io_descriptor.h"
#include "channel.h"

namespace minotaur {

LOGGER_CLASS_IMPL_NAME(logger, IOHandler, "net.io_handler");
thread_local IOHandler* IOHandler::current_io_handler_ = NULL;


uint32_t IOHandler::HashMessage(
    const EventMessage& message, 
    uint32_t worker_count) {
  return IODescriptorFactory::GetVersion(message.descriptor_id) % worker_count;
}

IOHandler::IOHandler(IOService* io_service, StageType* stage)
    : io_service_(io_service)
    , stage_(stage)
    , timer_(5, 1000 * 60 * 60){
}


void IOHandler::OnLoopStart() {
  prctl(PR_SET_NAME, "io_handler");
  current_io_handler_ = this;
}

void IOHandler::OnPerLoop() {
  Timer::NodeType* timer_head = timer_.ProcessTimer();
  Timer::NodeType* current = timer_head;
  Timer::NodeType* next = NULL;
  while (current) {
    next = current->next;
    if (current->active) {
      current->data();
    }
    current = next;
  }
  if (timer_head) {
    timer_.DestroyTimerNode(timer_head);
  }
}

void IOHandler::OnIdle() {
  //LOG_TRACE(logger, "IOHandler::OnIdle");
}

void IOHandler::Handle(const EventMessage& message) {
  MI_LOG_TRACE(logger, "IOHandler::Handle " << message);

  switch (message.type_id) {
    case minotaur::MessageType::kIOEvent:
      {
        if (message.payload & event::EventType::EV_READ) {
          HandleIOReadEvent(message);
        }
        if (message.payload & event::EventType::EV_WRITE) {
          HandleIOWriteEvent(message);
        }
        if (message.payload & event::EventType::EV_CLOSE) {
          HandleIOCloseEvent(message);
        }
        break;
      }
    case minotaur::MessageType::kIOMessageEvent:
      return HandleIOMessageEvent(message);
    default:
      MI_LOG_ERROR(logger, "IOHanlder unknown message:" << message.type_id);
      return;
  }
}

void IOHandler::HandleIOReadEvent(const EventMessage& message) {
  MI_LOG_TRACE(logger, "HandleIOReadEvent");
  IODescriptor* descriptor = 
      IODescriptorFactory::GetIODescriptor(message.descriptor_id);
  if (!descriptor) {
    MI_LOG_DEBUG(logger, "HandleReadEvent descriptor not found:"
        << message.descriptor_id);
    return;
  }

  descriptor->OnRead();
}

void IOHandler::HandleIOWriteEvent(const EventMessage& message) {
  MI_LOG_TRACE(logger, "HandleIOWriteEvent");
  IODescriptor* descriptor = 
      IODescriptorFactory::GetIODescriptor(message.descriptor_id);
  if (!descriptor) {
    MI_LOG_DEBUG(logger, "HandleWriteEvent descriptor not found:"
        << message.descriptor_id);
    return;
  }

  descriptor->OnWrite();
}

void IOHandler::HandleIOCloseEvent(const EventMessage& message) {
  MI_LOG_TRACE(logger, "HandleIOCloseEvent");
  IODescriptor* descriptor = 
      IODescriptorFactory::GetIODescriptor(message.descriptor_id);
  if (!descriptor) {
    MI_LOG_DEBUG(logger, "HandleIOCloseEvent descriptor not found:"
        << message.descriptor_id);
    return;
  }

  descriptor->OnClose();
}

void IOHandler::HandleIOMessageEvent(const EventMessage& message) {
  MI_LOG_TRACE(logger, "HandleIOMessageEvent");
  IODescriptor* descriptor = 
      IODescriptorFactory::GetIODescriptor(message.descriptor_id);
  if (!descriptor) {
    MI_LOG_DEBUG(logger, "HandleIOCloseEvent descriptor not found:"
        << message.descriptor_id);
    message.Destroy();
    return;
  }

  Channel* channel = dynamic_cast<Channel*>(descriptor);
  if (!channel) {
    MI_LOG_ERROR(logger, "IOHandler::HandleIOMessageEvent descriptor is not channel");
    message.Destroy();
    return;
  }

  ProtocolMessage* protocol_message = message.GetProtocolMessage();
  if (0 != channel->EncodeMessage(protocol_message)) {
    message.Destroy();
    return;    
  }

  descriptor->OnWrite();
}

IOHandlerFactory::IOHandlerFactory(IOService* io_service) 
    : io_service_(io_service) {
}

IOHandler* IOHandlerFactory::Create(StageType* stage) {
  return new IOHandler(io_service_, stage);
}


} //namespace minotaur
