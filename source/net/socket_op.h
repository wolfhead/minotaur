#ifndef _MINOTAUR_NET_SOCKET_OP_H_
#define _MINOTAUR_NET_SOCKET_OP_H_
/**
 * @file socket_op.h
 * @author Wolfhead
 */
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <string>
#include "../common/system_error.h"

namespace minotaur {

class SocketOperation {
 public:

  inline static int Read(int fd, void* buffer, uint32_t size) {
    return read(fd, buffer, size); 
  }

  inline static bool WouldBlock(int error) {
    return error == EAGAIN || error == EINTR;
  }

  inline static int SetNonBlocking(int fd) {
    return SetFlag(fd, O_NONBLOCK);
  }

  inline static int SetReuseAddr(int fd) {
    int on = 1;
    return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
  }

  inline static int SetFlag(int fd, int flag) {
    int flags = fcntl(fd, F_GETFL);
    if (flags == -1) {
      return -1;
    }
    return fcntl(fd, F_SETFL, flags | flag);
  }

  inline static int GetSocketAddress(
      const std::string& host, 
      int port,
      struct sockaddr_in* addr) {
    memset(addr, 0, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    addr->sin_addr.s_addr = htonl(INADDR_ANY);
    if (!host.empty() && inet_aton(host.c_str(), &addr->sin_addr) == 0) {
      return -1;
    }
    return 0;
  }

  inline static int CreateTcpSocket() {
    return socket(AF_INET, SOCK_STREAM, 0);
  }

  inline static int Bind(int fd, const struct sockaddr_in& addr) {
    return bind(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
  }

  inline static int Listen(int fd) {
    return listen(fd, SOMAXCONN);
  }

  inline static int Accept(int fd, struct sockaddr_in* addr) {
    socklen_t salen = sizeof(struct sockaddr_in);
    for (;;) {
      int client_fd = accept(fd, (struct sockaddr*)addr, &salen);
      if (client_fd == -1) {
        if (SystemError::Get() == EINTR) {
          continue;
        }
        break;
      }
      return client_fd;
    }
    return -1;
  }

  inline static int Receive(int fd, void* buffer, uint32_t bytes) {
    return read(fd, buffer, bytes);
  }

  inline static int Send(int fd, const void* buffer, uint32_t bytes) {
    return write(fd, buffer, bytes);
  }
};

} //namespace minotaur

#endif // _MINOTAUR_NET_SOCKET_OP_H_
