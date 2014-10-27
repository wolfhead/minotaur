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

  void Close();

  void Dump(std::ostream& os) const;

  std::string ToString() const;

 protected:
  static void SocketCommonProc(
      event::EventLoop* event_loop,
      int fd,
      void* data,
      uint32_t mask); 

  int RegisterRead();

  int RegisterWrite(); 

  virtual void OnRead(event::EventLoop* event_loop);

  virtual void OnWrite(event::EventLoop* event_loop);

  virtual void OnClose(event::EventLoop* event_loop);

  virtual void OnProcFinish(event::EventLoop* event_loop);

  IOService* io_service_;
  int fd_;
  bool should_close_;
 private:
  LOGGER_CLASS_DECL(logger);
};

std::ostream& operator << (std::ostream& os, const Socket& sock);

} //namespace minotaur

#endif //_MINOTAUR_NET_SOCKET_H_
