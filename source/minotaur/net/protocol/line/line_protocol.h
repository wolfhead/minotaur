#ifndef _MINOTAUR_LINE_PROTOCOL_H_
#define _MINOTAUR_LINE_PROTOCOL_H_ 
/**
 * @file line_protocol.h
 * @author Wolfhead
 */
#include "../protocol.h"
#include "../../../common/logger.h"

namespace ade {

class LineProtocol : public Protocol {
 public:
  LineProtocol();

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
    return this;
  }

  virtual void Destroy() {
  } 

 private:
  LOGGER_CLASS_DECL(logger);
};

} //namespace ade

#endif //_MINOTAUR_LINE_PROTOCOL_H_
