#ifndef _MINOTAUR_NET_SOCKET_H_
#define _MINOTAUR_NET_SOCKET_H_
/**
 * @file socket.h
 * @author Wolfhead
 */
#include <functional>
#include <boost/noncopyable.hpp>
#include "../common/logger.h"
#include "../event/event_loop.h"

namespace minotaur {

class IOService;

class Socket : public boost::noncopyable {
 public:
  typedef std::function<void(event::EventLoop* event_loop)> ReadProc;
  typedef std::function<void(event::EventLoop* event_loop)> WriteProc;

  Socket(IOService* io_service);
  Socket(IOService* io_service, int fd);
  virtual ~Socket();

  inline IOService* GetIOService() const {
    return io_service_;
  }

  inline void SetFD(int fd) {
    fd_ = fd;
  }

  inline int GetFD() const {
    return fd_;
  }

  inline void SetReadProc(const ReadProc& read_proc) {
    read_proc_ = read_proc;
  }

  inline void SetWriteProc(const WriteProc& write_proc) {
    write_proc_ = write_proc;
  }

 protected:
  static void SocketCommonProc(
      event::EventLoop* event_loop,
      int fd,
      void* data,
      uint32_t mask); 

  void Close(event::EventLoop* event_loop);

  int RegisterRead();

  int RegisterWrite(); 

  virtual void OnRead(event::EventLoop* event_loop);

  virtual void OnWrite(event::EventLoop* event_loop);

  virtual void OnProcFinish(event::EventLoop* event_loop);

  IOService* io_service_;
  int fd_;
  bool should_close_;
  ReadProc read_proc_;
  WriteProc write_proc_;
 private:
  LOGGER_CLASS_DECL(logger);
};

} //namespace minotaur

#endif //_MINOTAUR_NET_SOCKET_H_
