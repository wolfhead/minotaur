#ifndef MINOTAUR_ONLINE_DEBUG_H
#define MINOTAUR_ONLINE_DEBUG_H
/**
 * @file online_debug.h
 * @author Wolfhead
 */
#include <stdint.h>

namespace ade { namespace online_debug {

class OnlineDebug {
 public:
  static int Init();
  static void SetOnlineDebug(uint64_t id);
  static const char* GetOnlineDebugTrace();
  static uint64_t GetOnlineDebug();

 private:
  static bool init_;
};

} //namespace online_debug
} //namespace ade

#endif // MINOTAUR_ONLINE_DEBUG_H
