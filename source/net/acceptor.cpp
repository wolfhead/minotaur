/**
 * @file acceptor.cpp
 * @author Wolfhead
 */
#include "acceptor.h"
#include "socket_op.h"
#include "channel.h"
#include "../io_service.h"

namespace minotaur {

LOGGER_CLASS_IMPL_NAME(logger, Acceptor, "net.Acceptor");

Acceptor::Acceptor(IOService* io_service) 
    : Socket(io_service) {
}

Acceptor::~Acceptor() {
}

int Acceptor::Accept(const std::string& host, int port) {
  struct sockaddr_in sa;

  if (0 != SocketOperation::GetSocketAddress(host, port, &sa)) {
    MI_LOG_ERROR(logger, "Acceptor::Accept GetSocketAddress failed"
        << ", host:" << host
        << ", port:" << port);
    return -1;
  }

  int fd = SocketOperation::CreateTcpSocket();
  if (fd < 0) {
    MI_LOG_ERROR(logger, "Acceptor::Accept CreateTcpSocket, failed with:" 
        << SystemError::FormatMessage());
    return -1;
  }

  if (0 != SocketOperation::SetReuseAddr(fd)) {
    MI_LOG_ERROR(logger, "Acceptor::Accept SetReuseAddr, failed with:" 
        << SystemError::FormatMessage());
    close(fd);
    return -1;
  }

  if (0 != SocketOperation::SetNonBlocking(fd)) {
    MI_LOG_ERROR(logger, "Acceptor::Accept SetNonBlocking, failed with:" 
        << SystemError::FormatMessage());
    close(fd);
    return -1;
  }

  if (0 != SocketOperation::Bind(fd, sa)) {
    MI_LOG_ERROR(logger, "Acceptor::Accept bind, failed with:" 
        << SystemError::FormatMessage());
    close(fd);
    return -1;
  }
  
  if (0 != SocketOperation::Listen(fd)) {
    MI_LOG_ERROR(logger, "Acceptor::Accept listen, failed with:"
        << SystemError::FormatMessage());
    close(fd);
    return -1;
    }

  SetFD(fd);

  if (0 != RegisterRead()) {
    MI_LOG_ERROR(logger, "Acceptor::Accept RegisterRead fail");
    SetFD(-1);
    close(fd);
    return -1;
  }

  MI_LOG_TRACE(logger, "Acceptor::Accept Regsitered:" << *this);

  return 0;
}

void Acceptor::OnRead(event::EventLoop* event_loop) {
  struct sockaddr_in sa;

  while (true) {
    memset(&sa, 0, sizeof(sa));
    int client_fd = SocketOperation::Accept(GetFD(), &sa);
    if (client_fd < 0) {
      if (SocketOperation::WouldBlock(SystemError::Get())) {
        return;
      }
      MI_LOG_ERROR(logger, "Acceptor::OnRead accept failed with:"
          << SystemError::FormatMessage());  
      return;
    }

    char client_ip_buffer[32] = {0};
    int port = ntohs(sa.sin_port);
    inet_ntop(AF_INET, &sa.sin_addr, client_ip_buffer, INET_ADDRSTRLEN);

    Channel* channel = GetIOService()->CreateChannel(client_fd);
    if (!channel) {
      MI_LOG_ERROR(logger, "Acceptor::OnRead Create channel failed");
      continue;
    }

    channel->SetIp(client_ip_buffer);
    channel->SetPort(port);

    MI_LOG_TRACE(logger, "Acceptor::OnRead client connected on channel:"
        << channel->GetDiagnositicInfo());

    if (0 != channel->Start()) {
      MI_LOG_WARN(logger, "Acceptor::OnRead Channel Start fail");
      GetIOService()->DestoryChannel(channel->GetChannelId());
    }
  }
}

} //namespace minotaur
