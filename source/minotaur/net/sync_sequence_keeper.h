#ifndef _MINOTAUR_SYNC_SEQUENCE_KEEPER_H_
#define _MINOTAUR_SYNC_SEQUENCE_KEEPER_H_
/**
 * @file sync_sequence_keeper.h
 * @author Wolfhead
 */
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include "../common/logger.h"
#include "../queue/linked_list.h"

namespace ade {

class ProtocolMessage;

class SyncSequenceKeeper {
 public:
  typedef LinkedList<ProtocolMessage> QueueType;

  SyncSequenceKeeper(uint32_t timeout_in_msec);

  ~SyncSequenceKeeper();

  int Register(ProtocolMessage* message);

  ProtocolMessage* Fetch();

  ProtocolMessage* Fired();

  ProtocolMessage* Fire();

  QueueType Timeout();

  QueueType Clear();

  inline uint32_t Size() const {return queue_.size();}
 
 private:
  LOGGER_CLASS_DECL(logger);

  uint32_t timeout_msec_;
  QueueType queue_;

  ProtocolMessage* fired_;
};


} //namespace ade

#endif //_MINOTAUR_SYNC_SEQUENCE_KEEPER_H_
