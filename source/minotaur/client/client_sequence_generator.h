#ifndef MINOTAUR_CLIENT_SEQUENCE_GENERATOR_H
#define MINOTAUR_CLIENT_SEQUENCE_GENERATOR_H
/**
 * @file client_sequence_generator.h
 * @author Wolfhead
 */
#include "../common/thread_id.h"

namespace ade {

class ClientSequenceGenerator {
 public:
  static uint32_t Generate(uint8_t hash) {
    static thread_local uint32_t seq_;
    return ((uint32_t)hash) << 24 | (seq_++ & 0x00FFffff);
  }
};

} //namespace ade

#endif //MINOTAUR_CLIENT_SEQUENCE_GENERATOR_H
