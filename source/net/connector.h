#ifndef _MINOTAUR_NET_CONNECTOR_H_
#define _MINOTAUR_NET_CONNECTOR_H_
/**
 * @file connector.h
 * @author Wolfhead
 */

#include "socket.h"
#include "socket_op.h"

namespace minotaur {

class Connector : public Socket {
 public:
  Connector(IOService* io_service, const std::string& host, int port);

  ~Connector();

  virtual int Start();

  virtual int Stop();

  inline const std::string& GetHost() const {return host_;}

  inline int GetPort() const {return port_;}

 private:
  LOGGER_CLASS_DECL(logger);

  virtual void OnWrite();

  std::string host_;
  int port_;
  struct sockaddr_in sock_addr_;
};

} //namespace socket

#endif //_MINOTAUR_NET_CONNECTOR_H_
