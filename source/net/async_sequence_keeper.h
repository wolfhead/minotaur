#ifndef _MINOTAUR_ASYNC_SEQUENCE_KEEPER_H_
#define _MINOTAUR_ASYNC_SEQUENCE_KEEPER_H_
/**
 * @file async_sequence_keeper.h
 * @author Wolfhead
 */
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <boost/unordered_map.hpp>
#include "../common/logger.h"
#include "../queue/double_linked_list.h"

namespace minotaur { 

class ProtocolMessage;

class AsyncSequenceKeeper {
 public:
  typedef DoubleLinkedList<ProtocolMessage> QueueType;

  AsyncSequenceKeeper(uint32_t timeout_in_msec);

  ~AsyncSequenceKeeper();

  int Register(ProtocolMessage* message);

  ProtocolMessage* Fetch(uint32_t sequence_id);

  QueueType Timeout();

  QueueType Clear();

  inline uint32_t GenerateSequenceId() {return ++current_sequence_;}

 private:
  LOGGER_CLASS_DECL(logger);

  static inline uint64_t GetTimestamp(
      uint32_t timeout_in_sec, 
      uint32_t timeout_in_usec) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_usec + timeout_in_usec) | 
      ((tv.tv_sec + timeout_in_sec) << 32);
  }

  uint32_t current_sequence_;
  uint32_t timeout_sec_;
  uint32_t timeout_usec_;

  boost::unordered_map<uint32_t, ProtocolMessage*> registery_;
  QueueType queue_;
};

} //namespace minotaur

#endif // _MINOTAUR_ASYNC_SEQUENCE_KEEPER_H_
