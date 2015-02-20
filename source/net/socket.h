#ifndef _MINOTAUR_NET_SOCKET_H_
#define _MINOTAUR_NET_SOCKET_H_
/**
 * @file socket.h
 * @author Wolfhead
 */
#include "io_descriptor.h"

namespace minotaur {

class IOService;
class Protocol;

class Socket : public IODescriptor {
 public:
  Socket(IOService* io_service, bool use_io_stage);
  Socket(IOService* io_service, int fd, bool use_io_stage);

  ~Socket();

  inline void SetFD(int fd) {
    in_ = out_ = fd;
  }

  inline int GetFD() const {
    return in_;
  }

  inline void SetProtocol(Protocol* protocol) {
    protocol_ = protocol;
  }

  inline Protocol* GetProtocol() {
    return protocol_;
  }

  virtual void Dump(std::ostream& os) const;

 protected:
  Protocol* protocol_;

 private:
  LOGGER_CLASS_DECL(logger);
};

} //namespace minotaur

#endif //_MINOTAUR_NET_SOCKET_H_
