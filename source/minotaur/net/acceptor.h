#ifndef _MINOTAUR_NET_ACCEPTOR_H_
#define _MINOTAUR_NET_ACCEPTOR_H_
/**
 * @file acceptor.h
 * @author Wolfhead
 */
#include "socket.h"

namespace ade { 

class Service;

class Acceptor : public Socket {
 public:
  Acceptor(
      IOService* io_service, 
      const std::string& host,
      int port,
      Service* service);

  ~Acceptor();

  virtual int Start();

  virtual int Stop();

  inline const std::string& GetHost() const {return host_;}

  inline int GetPort() const {return port_;}

  inline Service* GetService() {return service_;}

 protected:
  virtual void OnRead();

  virtual void OnClose();

 private:
  LOGGER_CLASS_DECL(logger);

  std::string host_;
  int port_;
  Service* service_;
};

} //namespace ade

#endif // _MINOTAUR_NET_ACCEPTOR_H_
