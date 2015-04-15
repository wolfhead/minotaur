#ifndef _MINOTAUR_PROTOCOL_PROTOCOL_H_
#define _MINOTAUR_PROTOCOL_PROTOCOL_H_
/**
 * @file protocol.h
 * @author Wolfhead
 */
#include "protocol_factory.h"

namespace minotaur {

class ProtocolMessage;
class IODescriptor;
class IOBuffer;

class Protocol {
 public:
  enum {
    kResultContinue = 0,
    kResultDecoded = 1,
    kResultFail = 2,
  };

  Protocol(int protocol_type, bool is_sync) 
      : protocol_type_(protocol_type) 
      , is_sync_(is_sync) {}

  virtual ~Protocol() {};

  virtual ProtocolMessage* Decode(
      IODescriptor* descriptor, 
      IOBuffer* buffer, 
      int* result) = 0;

  virtual bool Encode(
      IODescriptor* descriptor,
      IOBuffer* buffer,
      ProtocolMessage* message) = 0;

  virtual ProtocolMessage* HeartBeatRequest() = 0;

  virtual ProtocolMessage* HeartBeatResponse(
      ProtocolMessage* request) = 0;

  virtual Protocol* Clone() = 0;
  virtual void Destroy() = 0;
  
  bool IsSync() const {return is_sync_;}
  int GetProtocolType() const {return protocol_type_;}

 protected:

  int protocol_type_;
  bool is_sync_;
};

} //namespace minotaur

#endif //_MINOTAUR_PROTOCOL_PROTOCOL_H_
