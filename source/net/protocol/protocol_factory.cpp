/**
 * @file protocol_factory.cpp
 * @author Wolfhead
 */
#include "protocol_factory.h"
#include "protocol.h"
#include "line/line_protocol.h"
#include "http/http_protocol.h"

namespace minotaur {

LOGGER_CLASS_IMPL_NAME(logger, ProtocolFactory, "net.ProtocolFactory");

ProtocolFactory::ProtocolFactory() {
  RegisterProtocol(ProtocolType::kLineProtocol, new LineProtocol());
  RegisterProtocol(ProtocolType::kHttpProtocol, new HttpProtocol());
}

ProtocolFactory::~ProtocolFactory() {
  for (auto protocol : protocol_) {
    if (protocol) delete protocol;
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

} //namespace minotaur
