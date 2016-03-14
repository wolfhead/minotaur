#ifndef _MINOTAUR_IO_REDIS_MESSAGE_H_
#define _MINOTAUR_IO_REDIS_MESSAGE_H_
/**
 * @file io_redis_message.h
 * @author Wolfhead
 */
#include "io_message.h"
#include "protocol/redis/redis_parser.h"

namespace ade {

class RedisProtocol;
class RedisResponseMessage;

class RedisRequestMessage : public ProtocolMessage {
 public:
  typedef RedisResponseMessage ResponseType;

  RedisRequestMessage() : command_count(0) {
    type_id = MessageType::kRedisRequestMessage;
  }

  void Ping();
  void Set(const std::string& key, const std::string& value);
  void Get(const std::string& key); 
  void Delete(const std::string& key);
  void Expire(const std::string& key, uint32_t expire);
  void SetEx(const std::string& key, const std::string& value, uint32_t expire);

  void Incr(const std::string& key);
  void IncrBy(const std::string& key, int by);

  void MGet(uint32_t size);
  void MGetAppend(const std::string& key);

  void MSet(uint32_t size);
  void MSetAppend(const std::string& key, const std::string& value);

  void HSET(const std::string hash, const std::string& key, const std::string& value);
  void HGETALL(const std::string& key);

  void Select(const std::string& db);

  virtual void Dump(std::ostream& os) const;

  uint32_t command_count;
  std::string body;
};

class RedisResponseMessage : public ProtocolMessage {
 public:
  friend RedisProtocol;

  RedisResponseMessage(uint32_t count) : current_(0), read_offset_(0) {
    type_id = MessageType::kRedisRequestMessage;
    resp_.resize(count);
  }

  uint32_t Size() {return resp_.size() - read_offset_;}
  const redis::RESP& At(uint32_t index) const {return resp_.at(index + read_offset_);}

  void PopFront() {
    if (read_offset_ != resp_.size()) {
      ++read_offset_;
    }
  }

  virtual void Dump(std::ostream& os) const;
 private:

  redis::RESP* CurrentRESP() {return &resp_[current_];}
  bool FinishAll() const {return current_ == resp_.size();}
  void FinishOne() {++current_;}

  std::vector<redis::RESP> resp_;
  uint32_t current_;
  uint32_t read_offset_;
};


std::ostream& operator << (std::ostream& os, const RedisRequestMessage& message);
std::ostream& operator << (std::ostream& os, const RedisResponseMessage& message);

}//namespace ade

#endif // _MINOTAUR_IO_REDIS_MESSAGE_H_
