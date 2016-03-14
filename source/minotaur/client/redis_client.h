#ifndef _MINOTAUR_REDIS_CLIENT_H_
#define _MINOTAUR_REDIS_CLIENT_H_
/**
 * @file redis_client.h
 * @author Wolfhead
 */
#include "../common/logger.h"

namespace ade {

class ClientRouter;
class RedisRequestMessage;
class RedisResponseMessage;

class RedisClient {
 public:
  RedisClient(ClientRouter* client, int db = 0);
  ~RedisClient();

  int Set(const std::string& key, const std::string& value);
  int SetEx(const std::string& key, const std::string& value, uint64_t expire);

  int Incr(const std::string& key);
  int IncrEx(const std::string& key, uint64_t expire);

  int Get(const std::string& key, std::string* value);
  int MGet(
      const std::vector<std::string>& key,
      std::map<std::string, std::string>* value);
 private:
  LOGGER_CLASS_DECL(logger);

  RedisRequestMessage* BuildRequest(); 
  RedisResponseMessage* GetResponse(RedisRequestMessage* request);

  ClientRouter* client_;
  std::string db_;
};

};

#endif //_MINOTAUR_REDIS_CLIENT_H_
