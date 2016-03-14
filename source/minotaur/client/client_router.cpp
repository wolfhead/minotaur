/**
 * @file client_router.cpp
 * @author Wolfhead
 */
#include "client_router.h"
#include <algorithm>
#include "../matrix/matrix_scope.h"

namespace ade {

LOGGER_CLASS_IMPL(logger, ClientRouter);

ClientRouter::ClientRouter(IOService* io_service, const std::string& name) 
    : io_service_(io_service)
    , name_(name) {
}

ClientRouter::~ClientRouter() {
  Stop();
  for (Client* client : clients_) {
    delete client;
  }
  for (ClientRouter* client : hash_) {
    delete client;
  }
}

int ClientRouter::AddClient(const std::string& address, uint32_t timeout_ms, uint32_t heartbeat_ms) {
  clients_.push_back(new Client(GetIOService(), name_, address, timeout_ms, heartbeat_ms));
  return 0;
}

int ClientRouter::AddClientRouter(ClientRouter* router) {
  hash_.push_back(router);
  return 0;
}

int ClientRouter::Start() {
  for (ClientRouter* client : hash_) {
    if (0 != client->Start()) {
      return -1;
    }
  }

  std::random_shuffle(clients_.begin(), clients_.end());

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
    MessageFactory::Destroy(message);
    return false;
  }
  return client->Send(message);
}

bool ClientRouter::SendHash(ProtocolMessage* message, uint32_t hash) {
  ClientRouter* client = GetHashClient(hash);
  if (!client) {
    MessageFactory::Destroy(message);
    return false;
  }
  return client->Send(message);
}

ProtocolMessage* ClientRouter::DoSendRecieve(ProtocolMessage* message, uint32_t timeout_ms) {
  matrix::MatrixScope matrix_scope(GetName(), matrix::MatrixScope::kModeAutoFail);
  Client* client = GetNextClient();
  if (!client) {
    LOG_DEBUG(logger, "ClientRouter::DoSendRecieve no avaliable client:" << GetName());
    MessageFactory::Destroy(message);
    return NULL;
  }
  
  ProtocolMessage* response = client->SendRecieve(message, timeout_ms);
  if (response) {
    matrix_scope.SetOkay(true);
  }
  return response;
}

ProtocolMessage* ClientRouter::DoSendRecieveHash(ProtocolMessage* message, uint32_t hash, uint32_t timeout_ms) {
  ClientRouter* client = GetHashClient(hash);
  return client->SendRecieve(message, timeout_ms);
}

Client* ClientRouter::GetNextClient() {
  for (size_t i = 0; i != std::min(clients_.size(), (size_t)5); ++i) {
    uint32_t current = (++current_ % clients_.size());
    if (clients_[current]->GetStatus() == Client::kWorking) {
      return clients_[current];
    } 
  }
  return NULL;
}

ClientRouter* ClientRouter::GetHashClient(uint32_t hash) {
  uint32_t current = hash % hash_.size();
  return hash_[current];
} 

std::string ClientRouter::Diagnostic() const {
  std::ostringstream oss;
  oss << "[";
  bool first = true;
  if (!hash_.empty()) {
    for (const auto& pair : hash_) {
      if (first) {
        first = false;
      } else {
        oss << ", ";
      }
      oss << pair->Diagnostic();
    }
  } else {
    for (const auto& pair : clients_) {
      if (first) {
        first = false;
      } else {
        oss << ", ";
      }
      oss << *pair;
    }
  }
  oss << "]";
  return oss.str();
}

} //namespace ade
