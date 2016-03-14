#ifndef _MINOTAUR_PROTOCOL_PROTOCOL_H_
#define _MINOTAUR_PROTOCOL_PROTOCOL_H_
/**
 * @file protocol.h
 * @author Wolfhead
 */
#include "protocol_factory.h"

namespace ade {

class ProtocolMessage;
class IOBuffer;

class Protocol {
 public:
  enum {
    kDecodeSuccess = 0,
    kDecodeContinue = 1,
    kDecodeFail = 2,
  };

  enum {
    kEncodeSuccess = 0,
    kEncodeFail = 1,
  };

  Protocol(int protocol_type, bool is_sync) 
      : protocol_type_(protocol_type) 
      , is_sync_(is_sync) {}

  virtual ~Protocol() {};

  virtual ProtocolMessage* Decode(
      IOBuffer* buffer, 
      int* result,
      ProtocolMessage* hint) = 0;

  virtual int Encode(
      IOBuffer* buffer,
      ProtocolMessage* message) = 0;

  virtual ProtocolMessage* HeartBeatRequest() = 0;

  virtual ProtocolMessage* HeartBeatResponse(
      ProtocolMessage* request) = 0;

  virtual Protocol* Clone() = 0;
  virtual void Destroy() = 0;
  
  virtual void Reset() {};

  bool IsSync() const {return is_sync_;}
  int GetProtocolType() const {return protocol_type_;}

 protected:

  int protocol_type_;
  bool is_sync_;
};

} //namespace ade

#endif //_MINOTAUR_PROTOCOL_PROTOCOL_H_
