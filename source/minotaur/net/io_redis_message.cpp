/**
 * @file io_redis_message.cpp
 * @author Wolfhead
 */
#include "io_redis_message.h"

namespace ade {

void RedisRequestMessage::Ping() {
  static const std::string& k_ping("*1\r\n$4\r\nPING\r\n");
  ++command_count;
  redis::RedisParser::EncodeRaw(k_ping, &body);
}

void RedisRequestMessage::Set(const std::string& key, const std::string& value) {
  static const std::string& k_raw("*3\r\n$3\r\nSET\r\n");
  ++command_count;
  redis::RedisParser::EncodeRaw(k_raw, &body);
  redis::RedisParser::EncodeBulkString(key, &body);
  redis::RedisParser::EncodeBulkString(value, &body);
}

void RedisRequestMessage::Get(const std::string& key) {
  static const std::string& k_raw("*2\r\n$3\r\nGET\r\n");
  ++command_count;
  redis::RedisParser::EncodeRaw(k_raw, &body);
  redis::RedisParser::EncodeBulkString(key, &body);
}

void RedisRequestMessage::Delete(const std::string& key) {
  static const std::string& k_raw("*2\r\n$3\r\nDEL\r\n");
  ++command_count;
  redis::RedisParser::EncodeRaw(k_raw, &body);
  redis::RedisParser::EncodeBulkString(key, &body);
}

void RedisRequestMessage::Expire(const std::string& key, uint32_t expire) {
  static const std::string& k_raw("*3\r\n$6\r\nEXPIRE\r\n");
  ++command_count;
  redis::RedisParser::EncodeRaw(k_raw, &body);
  redis::RedisParser::EncodeBulkString(key, &body);
  redis::RedisParser::EncodeBulkString(boost::lexical_cast<std::string>(expire), &body);
}

void RedisRequestMessage::SetEx(
    const std::string& key,
    const std::string& value,
    uint32_t expire) {
  static const std::string& k_raw("*4\r\n$5\r\nSETEX\r\n");
  ++command_count;
  redis::RedisParser::EncodeRaw(k_raw, &body);
  redis::RedisParser::EncodeBulkString(key, &body);
  redis::RedisParser::EncodeBulkString(boost::lexical_cast<std::string>(expire), &body);
  redis::RedisParser::EncodeBulkString(value, &body);
}

void RedisRequestMessage::Incr(
    const std::string& key) {
  static const std::string k_incr = "*2\r\n$4\r\nINCR\r\n";
  ++command_count;
  redis::RedisParser::EncodeRaw(k_incr, &body);
  redis::RedisParser::EncodeBulkString(key, &body);
}

void RedisRequestMessage::IncrBy(
    const std::string& key,
    int by) {
  static const std::string k_incrby = "*3\r\n$6\r\nINCRBY\r\n";
  ++command_count;
  redis::RedisParser::EncodeRaw(k_incrby, &body);
  redis::RedisParser::EncodeBulkString(key, &body);
  redis::RedisParser::EncodeBulkString(std::to_string(by), &body);
}

void RedisRequestMessage::MGet(uint32_t size) {
  static const std::string k_mget = "MGET";
  ++command_count;
  redis::RedisParser::EncodeArray(size + 1, &body);
  redis::RedisParser::EncodeBulkString(k_mget, &body);
}

void RedisRequestMessage::MGetAppend(const std::string& key) {
  redis::RedisParser::EncodeBulkString(key, &body);
}

void RedisRequestMessage::MSet(uint32_t size) {
  static const std::string k_mset = "MSET";
  ++command_count;
  redis::RedisParser::EncodeArray(size * 2 + 1, &body);
  redis::RedisParser::EncodeBulkString(k_mset, &body);
}

void RedisRequestMessage::MSetAppend(const std::string& key, const std::string& value) {
  redis::RedisParser::EncodeBulkString(key, &body);
  redis::RedisParser::EncodeBulkString(value, &body);
}

void RedisRequestMessage::HSET(const std::string hash, const std::string& key, const std::string& value) {
  static const std::string k_hset = "HSET";
  ++command_count;
  redis::RedisParser::EncodeArray(4, &body);
  redis::RedisParser::EncodeBulkString(k_hset, &body);
  redis::RedisParser::EncodeBulkString(hash, &body);
  redis::RedisParser::EncodeBulkString(key, &body);
  redis::RedisParser::EncodeBulkString(value, &body);
}

void RedisRequestMessage::HGETALL(const std::string& key) {
  static const std::string k_hgetall = "HGETALL";
  ++command_count;
  redis::RedisParser::EncodeArray(2, &body);
  redis::RedisParser::EncodeBulkString(k_hgetall, &body);
  redis::RedisParser::EncodeBulkString(key, &body);
}

void RedisRequestMessage::Select(const std::string& db) {
  static const std::string k_select = "SELECT";
  ++command_count;
  redis::RedisParser::EncodeArray(2, &body);
  redis::RedisParser::EncodeBulkString(k_select, &body);
  redis::RedisParser::EncodeBulkString(db, &body);
}

void RedisRequestMessage::Dump(std::ostream& os) const {
  os << "{\"type\": \"RedisRequestMessage\""
     << ", \"ProtocolMessage\":" ;
  ProtocolMessage::Dump(os);
  os << ", \"body\": \"" << body << "\""
     << ", \"command_count\": " << command_count
     << "}";
}

void RedisResponseMessage::Dump(std::ostream& os) const {
  os << "{\"type\": \"RedisResponseMessage\""
     << ", \"ProtocolMessage\":" ;
  ProtocolMessage::Dump(os);
  os << "[";
  bool first = true;
  for (const auto& resp : resp_) {
    if (first) {
      first = false;
    } else {
      os << ", ";
    }
    resp.Dump(os);
  }
  os << "]}";
}

std::ostream& operator << (std::ostream& os, const RedisRequestMessage& message) {
  message.Dump(os);
  return os;
}

std::ostream& operator << (std::ostream& os, const RedisResponseMessage& message) {
  message.Dump(os);
  return os;
}

} //namespace ade
