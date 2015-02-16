#ifndef MINOTUAR_CODEC_BASE_H
#define MINOTUAR_CODEC_BASE_H
/**
 * @file codec_base.h
 * @author Wolfhead
 */

#include "codec_factory.h"

namespace minotaur {

class IOBuffer;
class MessageBase;

class CodecBase {
 public:
  friend CodecFactory;

  virtual CodecBase* Clone() const;
  virtual void Destroy() = 0;

  virtual int Decode(IOBuffer* buffer, MessageBase** message) = 0;
  virtual int Encode(MessageBase* message, IOBuffer* buffer) = 0;
 protected:
  virtual ~CodecBase();
};

} //namespace minotaur

#endif // MINOTUAR_CODEC_BASE_H
