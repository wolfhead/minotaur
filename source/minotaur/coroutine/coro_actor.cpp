/**
 * @file coro_actor.cpp
 * @author Wolfhead
 */
#include "coro_actor.h"
#include "../net/io_message.h"
#include "../common/macro.h"
#include "coro_all.h"

namespace ade {

LOGGER_CLASS_IMPL(logger, CoroActor);

ProtocolMessage* CoroActor::RecieveMail(uint32_t timeout_ms) {
  uint64_t timer_id = 0;

  if (mailbox_.empty()) {
    if (timeout_ms) {
      timer_id = coro::StartTimer(timeout_ms);
    }
    Yield();
  }

  waiting_on_ = kNoMessage;
  coro::CancelTimer(timer_id);

  if (!mailbox_.empty()) {
    ProtocolMessage* message = mailbox_.front();
    mailbox_.pop_front();
    return message;
  } else {
    return NULL;
  }
}

bool CoroActor::SendMail(ProtocolMessage* message) {
  if (LIKELY(!waiting_on_ || (waiting_on_ && message->sequence_id == waiting_on_))) {
    mailbox_.push_back(message);
    waiting_on_ = kNoMessage;
    return true;
  } else {
    MI_LOG_DEBUG(logger, "CoroActor::SendMail seq mismatch"
        << ", waiting:" << waiting_on_
        << ", message:" << *message);
    return false;
  }
}

} //namespace ade
