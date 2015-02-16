/**
 * @file codec_factory.cpp
 * @author Wolfhead
 */
#include "codec_factory.h"
#include "codec_base.h"

namespace minotaur {

CodecFactory::CodecFactory() {
}

CodecFactory::~CodecFactory() {
  for (auto& codec : codec_) {
    if (codec) {
      delete codec;
      codec = NULL;
    }
  }
}

int CodecFactory::RegisterCodec(uint8_t codec_type, CodecBase* codec) {
  if (codec_.size() <= codec_type) {
    codec_.resize(codec_type + 1, NULL);
  }

  if (codec_[codec_type] != NULL) {
    return -1;
  }

  codec_[codec_type] = codec;
  return 0;
}

CodecBase* CodecFactory::GetCodec(uint8_t codec_type) {
  if (codec_type >= codec_.size()) {
    return NULL;
  }
  return codec_[codec_type];
}


} //namespace minotaur

