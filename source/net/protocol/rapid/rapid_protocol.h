#ifndef _MINOTAUR_RAPID_PROTOCOL_
#define _MINOTAUR_RAPID_PROTOCOL_
/**
 * @file rapid_protocol.h
 * @author Wolfhead
 */
#include "../protocol.h"
#include "../../../common/logger.h"

namespace minotaur {

class RapidProtocol : public Protocol {
 public:
  RapidProtocol();

  virtual ProtocolMessage* Decode(
      IODescriptor* descriptor, 
      IOBuffer* buffer, 
      int* result);

  virtual bool Encode(
      IODescriptor* descriptor,
      IOBuffer* buffer,
      ProtocolMessage* message);

  virtual ProtocolMessage* HeartBeatRequest();

  virtual ProtocolMessage* HeartBeatResponse(
      ProtocolMessage* request);

  virtual Protocol* Clone() {
    return this;
  }

  virtual void Destroy() {
  } 


 private:
  LOGGER_CLASS_DECL(logger);
};

} //namespace minotaur

#endif //_MINOTAUR_RAPID_PROTOCOL_
