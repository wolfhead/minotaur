#ifndef _MINOTAUR_COROUTINE_ACTOR_H_
#define _MINOTAUR_COROUTINE_ACTOR_H_ 
/**
 * @file coro_actor.h
 * @author Wolfhead
 */
#include "coroutine.h"
#include "../queue/linked_list.h"

namespace minotaur {

class IOService;
class ProtocolMessage;

class CoroActor : public Coroutine {
 public:
  ProtocolMessage* RecieveMail(uint32_t timeout_ms = 0);
  void SendMail(ProtocolMessage* message);

 private:
  LinkedList<ProtocolMessage> mailbox_;
};

} //namespace minotaur
#endif // _MINOTAUR_COROUTINE_ACTOR_H_
