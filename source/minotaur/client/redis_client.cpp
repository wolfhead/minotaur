/**
 * @file redis_client.cpp
 * @author Wolfhead
 */
#include "redis_client.h"
#include "client_router.h"
#include "../net/io_redis_message.h"

namespace ade {

LOGGER_CLASS_IMPL(logger, RedisClient);

RedisClient::RedisClient(ClientRouter* client, int db)
    : client_(client) 
    , db_(db ? std::to_string(db) : "") {
}

RedisClient::~RedisClient() {
}

int RedisClient::Set(const std::string& key, const std::string& value) {
  RedisRequestMessage* request = BuildRequest();
  request->Set(key, value);

  RedisResponseMessage* response = GetResponse(request);
  if (!response) {
    return -1;
  }
  if (response->Size() < 1 || response->At(0).IsError()) {
    MessageFactory::Destroy(response);
    return -1;
  }
  
  MessageFactory::Destroy(response);
  return 0;
}

int RedisClient::SetEx(const std::string& key, const std::string& value, uint64_t expire) {
  RedisRequestMessage* request = BuildRequest();
  request->SetEx(key, value, expire);

  RedisResponseMessage* response = GetResponse(request);
  if (!response) {
    return -1;
  }
  if (response->Size() < 1 || response->At(0).IsError()) {
    MessageFactory::Destroy(response);
    return -1;
  }
  
  MessageFactory::Destroy(response);
  return 0;
}

int RedisClient::Incr(const std::string& key) {
  RedisRequestMessage* request = BuildRequest();
  request->Incr(key);

  RedisResponseMessage* response = GetResponse(request);
  if (!response) {
    return -1;
  }
  if (response->Size() < 1 || response->At(0).IsError()) {
    MessageFactory::Destroy(response);
    return -1;
  }
  
  MessageFactory::Destroy(response);
  return 0;
}

int RedisClient::IncrEx(const std::string& key, uint64_t expire) {
  RedisRequestMessage* request = BuildRequest();
  request->Incr(key);
  request->Expire(key, expire);

  RedisResponseMessage* response = GetResponse(request);
  if (!response) {
    return -1;
  }
  if (response->Size() < 2 
      || response->At(0).IsError()
      || response->At(1).IsError()) {
    MessageFactory::Destroy(response);
    return -1;
  }
  
  MessageFactory::Destroy(response);
  return 0;
}

int RedisClient::Get(const std::string& key, std::string* value) {
  RedisRequestMessage* request = BuildRequest();
  request->Get(key);

  RedisResponseMessage* response = GetResponse(request);
  if (!response) {
    return -1;
  }
  if (response->Size() < 1 
      || !response->At(0).IsString()) {
    MessageFactory::Destroy(response);
    return -1;
  }
  
  value->assign(response->At(0).GetString());

  MessageFactory::Destroy(response);
  return 0;
}

int RedisClient::MGet(const std::vector<std::string>& key, std::map<std::string, std::string>* value) {
  RedisRequestMessage* request = BuildRequest();
  request->MGet(key.size());
  for (const auto& k : key) {
    request->MGetAppend(k);
  }

  RedisResponseMessage* response = GetResponse(request);
  if (!response) {
    return -1;
  }
  if (response->Size() < 1 
      || !response->At(0).IsArray()
      || response->At(0).Size() != key.size()) {
    MessageFactory::Destroy(response);
    return -1;
  }

  const auto& result = response->At(0);
  for (size_t i = 0; i != result.Size(); ++i) {
    if (result.At(i).IsString()) {
      (*value)[key[i]] = result.At(i).GetString();
    } 
  }
  
  MessageFactory::Destroy(response);
  return 0;
}

RedisRequestMessage* RedisClient::BuildRequest() {
  RedisRequestMessage* request = MessageFactory::Allocate<RedisRequestMessage>();
  if (!db_.empty()) {
    request->Select(db_);
  }
  return request;
}

RedisResponseMessage* RedisClient::GetResponse(RedisRequestMessage* request) {
  RedisResponseMessage* response = client_->SendRecieve(request);
  if (!response) {
    return NULL;
  }

  if (!db_.empty()) {
    if (response->Size() < 1) {
      LOG_WARN(logger, "RedisClient::GetResponse select internal failure");
      MessageFactory::Destroy(response);
      return NULL;
    }
       
    if ( response->At(0).IsError()) {
      LOG_WARN(logger, "RedisClient::GetResponse select fail:" << response->At(0).GetError());
      MessageFactory::Destroy(response);
      return NULL;
    } 

    response->PopFront();
  }

  return response;
}

} //namespace ade
