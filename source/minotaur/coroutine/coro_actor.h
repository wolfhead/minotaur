#ifndef _MINOTAUR_COROUTINE_ACTOR_H_
#define _MINOTAUR_COROUTINE_ACTOR_H_ 
/**
 * @file coro_actor.h
 * @author Wolfhead
 */
#include "coroutine.h"
#include "../queue/linked_list.h"
#include "../common/logger.h"

namespace ade {

class IOService;
class ProtocolMessage;

class CoroActor : public Coroutine {
 public:
  enum {
    kNoMessage = 0xFFFFFFFF,
    kAnyMessage = 0,
  };

  CoroActor() 
      : waiting_on_(kAnyMessage) {
  }

  ProtocolMessage* RecieveMail(uint32_t timeout_ms = 0);

  /**
   * @return true if this coro is ready to be scheduled
   *         false if not
   */
  bool SendMail(ProtocolMessage* message);

  inline void SetWaitingSequenceId(uint32_t sequence_id) {
    waiting_on_ = sequence_id;
  }

 private:
  LOGGER_CLASS_DECL(logger);

  ProtocolMessage* DoRecieveMail();

  LinkedList<ProtocolMessage> mailbox_;
  uint64_t waiting_on_;
};

} //namespace ade
#endif // _MINOTAUR_COROUTINE_ACTOR_H_
