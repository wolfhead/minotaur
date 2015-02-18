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
    , use_io_stage_(use_io_stage) {
}

IODescriptor::~IODescriptor() {
  Close();
}

void IODescriptor::IODescriptorProc(
    event::EventLoop* event_loop,
    int fd,
    void* data,
    uint32_t mask) {

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

  MI_LOG_TRACE(logger, "IODescriptorProc called"
      << ", fd:" << fd
      << ", data:" << data
      << ", mask:" << mask
      << ", descriptor_id" << desc->GetDescriptorId());

  if (desc->GetUseIOStage()) {
    int remove_mask = 
      mask & (event::EventType::EV_READ | event::EventType::EV_WRITE);
    event_loop->RemoveEvent(fd, remove_mask);
  }

  if (mask & event::EventType::EV_READ) {
    if (desc->GetUseIOStage()) {
      desc->SendIOMessage(IOMessage(
          MessageType::kIOReadEvent, desc->GetDescriptorId()));
    } else {
      desc->OnRead();    
    }
  } 

  if (mask & event::EventType::EV_WRITE) {
    if (desc->GetUseIOStage()) {
        desc->SendIOMessage(IOMessage(
          MessageType::kIOWriteEvent, desc->GetDescriptorId()));
    } else {
      desc->OnWrite();    
    }
  }

  if (mask & event::EventType::EV_CLOSE) {
    MI_LOG_TRACE(logger, "IODescriptor::IODescriptorProc EV_CLOSE, " << *desc);
    event_loop->DeleteEvent(fd);
    if (desc->UseIOStage()) {
        desc->SendIOMessage(IOMessage(
          MessageType::kIOCloseEvent, desc->GetDescriptorId()));
    } else {
      desc->OnClose();
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

int IODescriptor::SendIOMessage(const IOMessage& message) {
  if (!GetIOService()->GetIOStage()->Send(message)) {
    MI_LOG_ERROR(logger, "Channel::OnWrite send fail");
    return -1;
  }
  return 0;
}

void IODescriptor::OnRead() {
}

void IODescriptor::OnWrite() {
}

void IODescriptor::OnClose() {
  Close();
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
  GetIOService()->GetIODescriptorFactory()->Destroy(this);
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
