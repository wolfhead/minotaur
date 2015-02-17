/**
 * @file io_descriptor.cpp
 * @author Wolfhead
 */
#include "io_descriptor.h"
#include "../io_service.h"
#include "../message.h"
#include "../event/event_loop_stage.h"

namespace minotaur {

LOGGER_CLASS_IMPL_NAME(logger, IODescriptor, "net.IODescriptor");

IODescriptor::IODescriptor(IOService* io_service, int in, int out)
    : io_service_(io_service)
    , in_(in)
    , out_(out) {
}

IODescriptor::~IODescriptor() {
  Close();
}

void IODescriptor::IODescriptorProc(
    event::EventLoop* event_loop,
    int fd,
    void* data,
    uint32_t mask) {
  MI_LOG_TRACE(logger, "IODescriptorProc called"
      << ", fd:" << fd
      << ", data:" << data
      << ", mask:" << mask);

  if (!data) {
    MI_LOG_FATAL(logger, "IODescriptor::IODescriptorProc data is NULL");
    return;
  }

  IODescriptor* desc = static_cast<IODescriptor*>(data);
  if (desc->GetIN() != fd || desc->GetOUT() != fd) {
    MI_LOG_FATAL(logger, "IODescriptor::IODescriptorProc fd mismatch"
        << ", incoming:" << fd
        << ", current:" << *desc);
    return;
  }

  if (mask & event::EventType::EV_READ) {
    if (desc->UseIOStage()) {
      desc->SendIOMessage(MessageFactory::Allocate<IOMessageBase>(
          MessageType::kIOReadEvent, GetDescriptorId()));
    } else {
      desc->OnRead(event_loop);    
    }
  } 

  if (mask & event::EventType::EV_WRITE) {
    if (desc->UseIOStage()) {
        desc->SendIOMessage(MessageFactory::Allocate<IOMessageBase>(
          MessageType::kIOWriteEvent, GetDescriptorId()));
    } else {
      desc->OnRead(event_loop);    
    }
  }

  if (mask & event::EventType::EV_CLOSE) {
    MI_LOG_TRACE(logger, "IODescriptor::IODescriptorProc EV_CLOSE, " << *desc);
    event_loop->RemoveEvent(fd, 0xFFFFFFFF);
    if (desc->UseIOStage()) {
        desc->SendIOMessage(MessageFactory::Allocate<IOMessageBase>(
          MessageType::kIOCloseEvent, GetDescriptorId()));
    } else {
      desc->OnRead(event_loop);    
    }
    return;
  }
}

int IODescriptor::RegisterRead() {
  return GetIOService()->GetEventLoopStage()->RegisterRead(
      GetIN(), 
      &IODescriptor::IODescriptorProc,
      this);
}

int IODescriptor::RegisterWrite() {
  return GetIOService()->GetEventLoopStage()->RegisterWrite(
      GetOUT(),
      &IODescriptor::IODescriptorProc,
      this);
}

int IODescriptor::SendIOMessage(IOMessageBase* message) {
      IOMessageBase* msg =
  if (!GetIOService()->GetIOStage()->Send(msg)) {
    MI_LOG_ERROR(logger, "Channel::OnWrite send fail");
    MessageFactory::Destory(msg);
    return -1;
  }
  return 0;
}

void IODescriptor::OnRead(event::EventLoop* event_loop) {
}

void IODescriptor::OnWrite(event::EventLoop* event_loop) {
}

void IODescriptor::OnClose(event::EventLoop* event_loop) {
  if (in_ == out_) {
    close(in_);
  } else {
    close(out_);
    close(in_);
  }
  in_ = out_ = -1;
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
