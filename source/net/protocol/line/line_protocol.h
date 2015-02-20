#ifndef _MINOTAUR_LINE_PROTOCOL_H_
#define _MINOTAUR_LINE_PROTOCOL_H_ 
/**
 * @file line_protocol.h
 * @author Wolfhead
 */
#include "../protocol.h"
#include "../../../common/logger.h"

namespace minotaur {

class LineProtocol : public Protocol {
 public:
  LineProtocol();

  virtual ProtocolMessage* Decode(
      IODescriptor* descriptor, 
      IOBuffer* buffer, 
      int* result);

  virtual bool Encode(
      IODescriptor* descriptor,
      IOBuffer* buffer,
      ProtocolMessage* message);

  virtual Protocol* Clone() {
    return this;
  }

  virtual void Destroy() {
  } 


 private:
  LOGGER_CLASS_DECL(logger);
};

} //namespace minotaur

#endif //_MINOTAUR_LINE_PROTOCOL_H_
