#ifndef MINOTAUR_CODEC_FACTORY_H
#define MINOTAUR_CODEC_FACTORY_H
/**
 * @file codec_factory.h
 * @author Wolfhead
 */
#include <vector>

namespace minotaur {

class CodecBase;

class CodecFactory {
 public:
  static CodecFactory* Instance() {
    static CodecFactory instance_;
    return &instance_;
  }

  CodecBase* GetCodec(uint8_t codec_type);

  int RegisterCodec(uint8_t codec_type, CodecBase* codec);

 private:
  CodecFactory();
  ~CodecFactory();
  CodecFactory(const CodecFactory& factory);
  CodecFactory& operator = (const CodecFactory& factory);

  std::vector<CodecBase*> codec_;
};

} //namespace minotaur

#endif // MINOTAUR_CODEC_FACTORY_H
