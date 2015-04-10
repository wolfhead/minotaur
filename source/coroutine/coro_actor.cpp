/**
 * @file coro_actor.cpp
 * @author Wolfhead
 */
#include "coro_actor.h"
#include "../net/io_message.h"
#include "coro_all.h"

namespace minotaur {

ProtocolMessage* CoroActor::RecieveMail(uint32_t timeout_ms) {
  uint64_t timer_id = 0;

  if (mailbox_.empty()) {
    if (timeout_ms) {
      timer_id = coro::StartTimer(timeout_ms);
    }
    Yield();
  }

  coro::CancelTimer(timer_id);

  if (!mailbox_.empty()) {
    ProtocolMessage* message = mailbox_.front();
    mailbox_.pop_front();
    return message;
  } else {
    return NULL;
  }
}

void CoroActor::SendMail(ProtocolMessage* message) {
  mailbox_.push_back(message);
}


} //namespace minotaur
