#ifndef _MINOTAUR_NET_ACCEPTOR_H_
#define _MINOTAUR_NET_ACCEPTOR_H_
/**
 * @file acceptor.h
 * @author Wolfhead
 */
#include "socket.h"

namespace minotaur { 

class Acceptor : public Socket {
 public:
  Acceptor(IOService* io_service);

  ~Acceptor();

  int Accept(const std::string& ip, int port);

 protected:
  virtual void OnRead(event::EventLoop* event_loop);

 private:
  LOGGER_CLASS_DECL(logger);
};

} //namespace minotaur

#endif // _MINOTAUR_NET_ACCEPTOR_H_
