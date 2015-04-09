/**
 * @file client_router.cpp
 * @author Wolfhead
 */
#include "client_router.h"

namespace minotaur {

LOGGER_CLASS_IMPL(logger, ClientRouter);

ClientRouter::ClientRouter(IOService* io_service) 
    : io_service_(io_service) {
}

ClientRouter::~ClientRouter() {
  Stop();
  for (Client* client : clients_) {
    delete client;
  }
}

int ClientRouter::AddClient(const std::string& address, uint32_t timeout_ms) {
  clients_.push_back(new Client(GetIOService(), address, timeout_ms));
  return 0;
}

int ClientRouter::Start() {
  for (Client* client : clients_) {
    if (0 != client->Start()) {
      return -1;
    }
  }
  return 0;
}

int ClientRouter::Stop() {
  for (Client* client : clients_) {
    client->Stop();
  }
  return 0;
}

bool ClientRouter::Send(ProtocolMessage* message) {
  Client* client = GetNextClient();
  if (!client) {
    return false;
  }
  return client->Send(message);
}

bool ClientRouter::SendHash(ProtocolMessage* message, uint32_t hash) {
  Client* client = GetHashClient(hash);
  if (!client) {
    return false;
  }
  return client->Send(message);
}

ProtocolMessage* ClientRouter::DoSendRecieve(ProtocolMessage* message, uint32_t timeout_ms) {
  Client* client = GetNextClient();
  if (!client) {
    return NULL;
  }
  return client->SendRecieve(message, timeout_ms);
}

ProtocolMessage* ClientRouter::DoSendRecieveHash(ProtocolMessage* message, uint32_t hash, uint32_t timeout_ms) {
  Client* client = GetHashClient(hash);
  if (!client) {
    return NULL;
  }
  return client->SendRecieve(message, timeout_ms);
}

Client* ClientRouter::GetNextClient() {
  return clients_[0];
  for (size_t i = 0; i != clients_.size(); ++i) {
    uint32_t current = (++current_ % clients_.size());
    if (clients_[current]->GetStatus() == Client::kWorking) {
      return clients_[current];
    } 
  }
  return NULL;
}

Client* ClientRouter::GetHashClient(uint32_t hash) {
  uint32_t current = hash % clients_.size();
  if (clients_[current]->GetStatus() == Client::kWorking) {
    return clients_[current];
  }
  return NULL;
} 

} //namespace minotaur
