/**
 * @file protocol_factory.cpp
 * @author Wolfhead
 */
#include "protocol_factory.h"
#include "protocol.h"
#include "line/line_protocol.h"
#include "rapid/rapid_protocol.h"
#include "http/http_protocol.h"
#include "redis/redis_protocol.h"
#include "memcached/memcached_protocol.h"
#include "../../matrix/matrix_mem_perf.h"

namespace ade {

LOGGER_CLASS_IMPL_NAME(logger, ProtocolFactory, "net.ProtocolFactory");

ProtocolFactory::ProtocolFactory() {
  RegisterProtocol(ProtocolType::kLineProtocol, ADE_NEW(LineProtocol));
  RegisterProtocol(ProtocolType::kRapidProtocol, ADE_NEW(RapidProtocol));
  RegisterProtocol(ProtocolType::kHttpProtocol, ADE_NEW(HttpProtocol));
  RegisterProtocol(ProtocolType::kRedisProtocol, ADE_NEW(RedisProtocol));
  RegisterProtocol(ProtocolType::kMemcachedProtocol, ADE_NEW(MemcachedProtocol));
}

ProtocolFactory::~ProtocolFactory() {
  for (auto protocol : protocol_) {
    if (protocol) ADE_DELETE(protocol);
  }
}

Protocol* ProtocolFactory::Create(int type) {
  if (type <= 0 || type >= (int)protocol_.size()) {
    return NULL;
  }

  Protocol* protocol = protocol_.at(type);
  if (!protocol) {
    return NULL;
  }

  return protocol->Clone();
}

void ProtocolFactory::RegisterProtocol(int type, Protocol* protocol) {
  if (type >= (int)protocol_.size()) {
    protocol_.resize(type + 1);
  }

  protocol_[type] = protocol;
}

} //namespace ade
