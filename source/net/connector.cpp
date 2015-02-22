/**
 * @file connector.cpp
 * @author Wolfhead
 */
#include "connector.h"
#include "socket_op.h"
#include "channel.h"
#include "../io_service.h"
#include "io_descriptor_factory.h"
#include "protocol/protocol.h"

namespace minotaur {

LOGGER_CLASS_IMPL_NAME(logger, Connector, "net.Connector");

Connector::Connector(IOService* io_service, const std::string& host, int port)
    : Socket(io_service, false) 
    , host_(host)
    , port_(port) {
}

Connector::~Connector() {
}

int Connector::Start() {
  if (0 != SocketOperation::GetSocketAddress(GetHost(), GetPort(), &sock_addr_)) {
    MI_LOG_ERROR(logger, "Connector::Start GetSocketAddress failed"
        << ", host:" << GetHost()
        << ", port:" << GetPort());
    return -1;
  }

  int fd = SocketOperation::CreateTcpSocket();
  if (fd < 0) {
    MI_LOG_ERROR(logger, "Connector::Start CreateTcpSocket, failed with:" 
        << SystemError::FormatMessage());
    return -1;
  }

  if (0 != SocketOperation::SetNonBlocking(fd)) {
    MI_LOG_ERROR(logger, "Connector::Start SetNonBlocking, failed with:" 
        << SystemError::FormatMessage());
    close(fd);
    return -1;
  }

  SetFD(fd);

  if (-1 != SocketOperation::Connect(fd, &sock_addr_)) {
    if (SystemError::Get() == EINPROGRESS) {
      MI_LOG_ERROR(logger, "Connector::Start Connect, in progress with:" 
          << SystemError::FormatMessage());
    } else {
      MI_LOG_ERROR(logger, "Connector::Start Connect, failed with:" 
          << SystemError::FormatMessage());
      return -1;
    }
  }

  if (0 != RegisterWrite()) {
    MI_LOG_ERROR(logger, "Connector::Start RegisterRead fail");
    SetFD(-1);
    close(fd);
    return -1;
  }

  return 0;
}

int Connector::Stop() {
  assert(!"not implement");
  return -1;
}

void Connector::OnWrite() {
  MI_LOG_DEBUG(logger, "Connector::OnWrite host:" << GetHost() << ", port:" << GetPort());

  char client_ip_buffer[32] = {0};
  int port = ntohs(sock_addr_.sin_port);
  inet_ntop(AF_INET, &sock_addr_.sin_addr, client_ip_buffer, INET_ADDRSTRLEN);

  Channel* channel = IODescriptorFactory::Instance()
      .CreateChannel(GetIOService(), GetFD());
  SetFD(-1);

  channel->SetIp(client_ip_buffer);
  channel->SetPort(port);
  channel->SetProtocol(GetProtocol()->Clone());
  
  MI_LOG_TRACE(logger, "Connector::OnWrite client connected on channel:"
      << channel->GetDiagnositicInfo());

  if (0 != channel->Start()) {
    MI_LOG_WARN(logger, "Connector::OnWrite Channel Start fail");
    channel->Destroy();
  }
}


} //namespace minotaur
