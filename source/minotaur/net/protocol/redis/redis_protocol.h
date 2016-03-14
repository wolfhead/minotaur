#ifndef _MINOTAUR_REDIS_PROTOCOL_H_
#define _MINOTAUR_REDIS_PROTOCOL_H_
/**
 * @file redis_protocol.h
 * @author Wolfhead
 */
#include "../protocol.h"
#include "../../../common/logger.h"
#include "../../../matrix/matrix_mem_perf.h"

namespace ade {

class RedisResponseMessage;

class RedisProtocol : public Protocol {
 public:
  RedisProtocol();

  ~RedisProtocol();

  virtual ProtocolMessage* Decode(
      IOBuffer* buffer,
      int* result,
      ProtocolMessage* hint);

  virtual int Encode(
      IOBuffer* buffer,
      ProtocolMessage* message);

  virtual Protocol* Clone() {return ADE_NEW(RedisProtocol);}

  virtual void Destroy() {ADE_DELETE(this);}

  virtual void Reset();

  virtual ProtocolMessage* HeartBeatRequest();

  virtual ProtocolMessage* HeartBeatResponse(ProtocolMessage* request);

 private:
  LOGGER_CLASS_DECL(logger);

  RedisResponseMessage* current_;
};
}

#endif //_MINOTAUR_REDIS_PROTOCOL_H_
