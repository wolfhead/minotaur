#ifndef _MINOTAUR_MEMCACHED_PROTOCOL_H_
#define _MINOTAUR_MEMCACHED_PROTOCOL_H_
/**
 * @file memcache_protocol.h
 * @author Wolfhead
 */
#include "../protocol.h"
#include "../../../common/logger.h"

namespace ade {

class LineMessage;

class MemcachedProtocol : public Protocol {
 public:
  MemcachedProtocol();
  ~MemcachedProtocol();

  virtual ProtocolMessage* Decode(
      IOBuffer* buffer, 
      int* result,
      ProtocolMessage* hint);

  virtual int Encode(
      IOBuffer* buffer,
      ProtocolMessage* message);

  virtual ProtocolMessage* HeartBeatRequest();

  virtual ProtocolMessage* HeartBeatResponse(ProtocolMessage* request);

  virtual Protocol* Clone() {
    return new MemcachedProtocol();
  }

  virtual void Destroy() {
    delete this;
  } 

  virtual void Reset();
 private:
  LOGGER_CLASS_DECL(logger);

  LineMessage* current_;
};

} //namespace ade

#endif //_MINOTAUR_MEMCACHED_PROTOCOL_H_
