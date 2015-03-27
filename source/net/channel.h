#ifndef _MINOTAUR_NET_CHANNEL_H_
#define _MINOTAUR_NET_CHANNEL_H_
/**
 * @file channel.h
 * @author Wolfhead
 */
#include "socket.h"
#include "io_buffer.h"

namespace minotaur { 

class ProtocolMessage;

class Channel : public Socket {
 public:
  friend IOHandler;

  Channel(IOService* io_service, int fd);

  ~Channel();

  virtual int Start();

  virtual int Stop();

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

 protected:
  virtual void OnRead();

  virtual void OnWrite();

  virtual void OnClose();

  virtual int DecodeMessage();

  virtual int EncodeMessage(ProtocolMessage* message);

  virtual void OnDecodeMessage(ProtocolMessage* message) = 0;

  std::string ip_;
  int port_;
  IOBuffer read_buffer_;
  IOBuffer write_buffer_;

 private:
  LOGGER_CLASS_DECL(logger);
};

} //namespace minotaur

#endif // _MINOTAUR_NET_CHANNEL_H_
