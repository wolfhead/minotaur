#ifndef _MINOTAUR_NET_IO_DESCRIPTOR_H_
#define _MINOTAUR_NET_IO_DESCRIPTOR_H_
/**
 * @file io_descriptor.h
 * @author Wolfhead
 */

#include <atomic>
#include <boost/noncopyable.hpp>
#include "../common/logger.h"
#include "../event/event_loop.h"

namespace minotaur {

class IOService;
class IOMessage;

class IODescriptor : public boost::noncopyable {
 public:
  IODescriptor(
      IOService* io_service, 
      int in, 
      int out, 
      bool use_io_stage);

  virtual ~IODescriptor();

  inline IOService* GetIOService() const {
    return io_service_;
  }

  inline int GetIN() const {return in_;}

  inline int GetOUT() const {return out_;}

  inline uint64_t GetDescriptorId() const {return descriptor_id_;}

  inline void SetDescriptorId(uint64_t id) {descriptor_id_ = id;}

  inline bool GetUseIOStage() const {return use_io_stage_;}

  inline void SetCloseMark() {close_mark_.store(true, std::memory_order_release);}

  inline bool GetCloseMark() {return close_mark_.load(std::memory_order_acquire);}

  virtual int Start() = 0;

  virtual int Stop() = 0;

  virtual void Close();

  // these functions might be called in either
  // EventLoopStage or IOStage
  // depending on GetUseIOStage
  virtual void OnRead();

  virtual void OnWrite();

  virtual void OnClose();

  void Destroy();

  virtual void Dump(std::ostream& os) const;

  std::string ToString() const;

 protected:
  int RegisterRead();

  int RegisterWrite(); 

  int RegisterReadWrite();

  int SendIOMessage(const IOMessage& message);

  bool UseIOStage() const {return use_io_stage_;}

  IOService* io_service_;
  uint64_t descriptor_id_;
  int in_;
  int out_;
  bool use_io_stage_;
  std::atomic<bool> close_mark_;

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
