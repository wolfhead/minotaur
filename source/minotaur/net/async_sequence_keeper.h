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

namespace ade { 

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

  uint32_t current_sequence_;
  uint32_t timeout_msec_;

  boost::unordered_map<uint32_t, ProtocolMessage*> registery_;
  QueueType queue_;
};

} //namespace ade

#endif // _MINOTAUR_ASYNC_SEQUENCE_KEEPER_H_
