#ifndef _MINOTAUR_NET_CHANNEL_H_
#define _MINOTAUR_NET_CHANNEL_H_
/**
 * @file channel.h
 * @author Wolfhead
 */
#include "socket.h"
#include "io_buffer.h"

namespace minotaur { 

class Channel : public Socket{
 public:
  Channel(IOService* io_service, int fd);

  ~Channel();

  int Start();

  inline void SetIp(const std::string& ip) {
    ip_ = ip;
  }
  const std::string GetIp() const {
    return ip_;
  }

  inline void SetPort(int port) {
    port_ = port;
  }
  inline int GetPort() const {
    return port_;
  }

  std::string GetDiagnositicInfo() const;

  void DumpDisgnosticInfo(std::ostream& os) const;

 private:
  LOGGER_CLASS_DECL(logger);

  void ReadBuffer();

  void WriteBuffer();

  virtual void OnRead(event::EventLoop* event_loop);

  virtual void OnWrite(event::EventLoop* event_loop);

  virtual void OnClose(event::EventLoop* event_loop);

  std::string ip_;
  int port_;
  IOBuffer read_buffer_;
  IOBuffer write_buffer_;
};

} //namespace minotaur

#endif // _MINOTAUR_NET_CHANNEL_H_
