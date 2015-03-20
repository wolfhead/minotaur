/**
 * @file io_descriptor.cpp
 * @author Wolfhead
 */
#include "io_descriptor.h"
#include "../io_service.h"
#include "../message.h"
#include "../event/event_loop_stage.h"
#include "../stage.h"
#include "io_handler.h"
#include "io_descriptor_factory.h"

namespace minotaur {

LOGGER_CLASS_IMPL_NAME(logger, IODescriptor, "net.IODescriptor");

IODescriptor::IODescriptor(
    IOService* io_service, 
    int in, 
    int out,
    bool use_io_stage)
    : io_service_(io_service)
    , in_(in)
    , out_(out)
    , use_io_stage_(use_io_stage)
    , close_mark_(false) {
}

IODescriptor::~IODescriptor() {
  Close();
}

void IODescriptor::IODescriptorProc(
    event::EventLoop* event_loop,
    int fd,
    void* data,
    uint32_t mask) {

  IODescriptor* desc = IODescriptorFactory::Instance()
      .GetIODescriptor((uint64_t)data);
  if (!desc) {
    MI_LOG_WARN(logger, "IODescriptor::IODescriptorProc descriptor not found:" 
        << (uint64_t)data << ", fd:" << fd << ", mask:" << mask);
    return;
  }

  MI_LOG_TRACE(logger, "IODescriptorProc called"
      << ", fd:" << fd
      << ", mask:" << mask
      << ", descriptor_id:" << desc->GetDescriptorId());

  if (mask & event::EventType::EV_CLOSE) {
    MI_LOG_TRACE(logger, "IODescriptor::IODescriptorProc EV_CLOSE, " 
        << *desc);
    event_loop->DeleteEvent(fd);
  } else if (desc->GetUseIOStage()) {
    //int remove_mask = 
    //  mask & (event::EventType::EV_READ | event::EventType::EV_WRITE);
    //event_loop->RemoveEvent(fd, remove_mask);
  }

  if (desc->GetUseIOStage()) {
    desc->SendEventMessage(EventMessage(
        MessageType::kIOEvent, desc->GetDescriptorId(), mask));
  } else {
    if (mask & event::EventType::EV_READ) {
      desc->OnRead();    
    } 
    if (mask & event::EventType::EV_WRITE) {
      desc->OnWrite();    
    }
    if (mask & event::EventType::EV_CLOSE) {
      desc->OnClose();
    }
  }
}

int IODescriptor::RegisterRead() {
  return GetIOService()->GetEventLoopStage()->RegisterRead(
      event::EventLoopNotifier::kDescriptorFD, 
      &IODescriptor::IODescriptorProc,
      (void*)GetDescriptorId());
}

int IODescriptor::RegisterWrite() {
  return GetIOService()->GetEventLoopStage()->RegisterWrite(
      event::EventLoopNotifier::kDescriptorFD,
      &IODescriptor::IODescriptorProc,
      (void*)GetDescriptorId());
}

int IODescriptor::RegisterReadWrite() {
  return GetIOService()->GetEventLoopStage()->RegisterReadWrite(
      event::EventLoopNotifier::kDescriptorFD,
      &IODescriptor::IODescriptorProc,
      (void*)GetDescriptorId());
}

int IODescriptor::RegisterClose() {
  return GetIOService()->GetEventLoopStage()->RegisterClose(
      event::EventLoopNotifier::kDescriptorFD, (void*)GetDescriptorId());
}

int IODescriptor::SendEventMessage(const EventMessage& message) {
  if (!GetIOService()->GetIOStage()->Send(message)) {
    MI_LOG_ERROR(logger, "Channel::SendEventMessage send fail, type:" 
        << (int)message.type_id);
    return -1;
  }
  return 0;
}

void IODescriptor::OnRead() {
  MI_LOG_TRACE(logger, "IODescriptor::OnRead:" << *this);
}

void IODescriptor::OnWrite() {
  MI_LOG_TRACE(logger, "IODescriptor::OnWrite:" << *this);
}

void IODescriptor::OnClose() {
  MI_LOG_TRACE(logger, "IODescriptor::OnClose:" << *this);
  Close();
}

void IODescriptor::OnActiveClose() {
  MI_LOG_TRACE(logger, "IODescriptor::OnActiveClose:" << *this);
  RegisterClose();
}

void IODescriptor::Close() {
  if (in_ == out_) {
    if (in_ != -1) close(in_);
  } else {
    if (in_ != -1) close(in_);
    if (out_ != -1) close(out_);
  }
  in_ = out_ = -1;
}

void IODescriptor::Destroy() {
  MI_LOG_TRACE(logger, "IODescriptor::Destroy " << *this);
  IODescriptorFactory::Instance().Destroy(this);
}

void IODescriptor::Dump(std::ostream& os) const {
  os << "[IODescriptor in:" << in_ << ", out:" << out_ << "]";
}

std::string IODescriptor::ToString() const {
  std::ostringstream oss;
  Dump(oss);
  return oss.str();
}

std::ostream& operator << (std::ostream& os, const IODescriptor& desc) {
  desc.Dump(os);
  return os;
}


} //namespace minotaur
