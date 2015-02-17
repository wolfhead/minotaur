#ifndef _MINOTAUR_NET_IO_DESCRIPTOR_H_
#define _MINOTAUR_NET_IO_DESCRIPTOR_H_
/**
 * @file io_descriptor.h
 * @author Wolfhead
 */

#include <boost/noncopyable.hpp>
#include "../common/logger.h"
#include "../event/event_loop.h"

namespace minotaur {

class IOService;
class IOMessageBase;

class IODescriptor : public boost::noncopyable {
 public:
  IODescriptor(IOService* io_service, int in, int out);
  virtual ~IODescriptor();

  inline IOService* GetIOService() const {
    return io_service_;
  }

  inline int GetIN() const {return in_;}

  inline int GetOUT() const {return out_;}

  inline uint64_t GetDescriptorId() const {return descriptor_id_;}

  inline void SetDescriptId(uint64_t id) {descriptor_id_ = id;}

  virtual void Close();

  virtual void Dump(std::ostream& os) const;

  std::string ToString() const;

 protected:
  int RegisterRead();

  int RegisterWrite(); 

  int SendIOMessage(IOMessageBase* message);

  bool UseIOStage() const {return use_io_stage_;}

  virtual void OnRead(event::EventLoop* event_loop);

  virtual void OnWrite(event::EventLoop* event_loop);

  virtual void OnClose(event::EventLoop* event_loop);

  IOService* io_service_;
  uint64_t descriptor_id_;
  int in_;
  int out_;
  bool use_io_stage_;

 private:
  static void IODescriptorProc(
      event::EventLoop* event_loop,
      int fd,
      void* data,
      uint32_t mask); 

  LOGGER_CLASS_DECL(logger);
};

std::ostream& operator << (std::ostream& os, const IODescriptor& desc); 

} //namespace minotaur

#endif //_MINOTAUR_NET_IO_DESCRIPTOR_H_
