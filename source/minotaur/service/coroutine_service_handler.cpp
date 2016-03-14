/**
 * @file coroutine_service_handler.cpp
 * @author Wolfhead
 */
#include "coroutine_service_handler.h"
#include <sys/prctl.h>
#include "../coroutine/coro_all.h"
#include "../service/service.h"
#include "../matrix/matrix_scope.h"
#include "../common/macro.h"

namespace ade {

LOGGER_CLASS_IMPL(logger, CoroutineServiceHandler);

CoroutineServiceHandler::CoroutineServiceHandler(
    IOService* io_service) 
    : ServiceHandler(io_service) 
    , timer_(1, 1000 * 60 * 60 * 24) {
}

void CoroutineServiceHandler::Run(StageData<ServiceHandler>* data) {
  prctl(PR_SET_NAME, "coro_service_handler");

  CoroBootstrap bootstrap;
  CoroutineContext::Init(
      ThreadLocalCorotineFactory::Instance(),
      &timer_,
      GetIOService(),
      this);

  CoroScheduler* scheduler = coro::Spawn<CoroScheduler>();
  coro::SpawnAndSchedule<CoroTask>(
      boost::bind(&CoroutineServiceHandler::Process, this, data));

  bootstrap.Start(scheduler);
}

void CoroutineServiceHandler::Process(StageData<ServiceHandler>* data) {
  while (data->running) {
    ProcessTimer();
    ProcessMessage(data);

    coro::Schedule(coro::Current());
    coro::Yield();
  }
}

void CoroutineServiceHandler::ProcessTimer() {
  Timer::NodeType* timer_header = timer_.ProcessTimer();
  Timer::NodeType* current = timer_header;
  while (current) {
    if (current->active) {
      CoroActor* actor = coro::GetCoroutine<CoroActor>(current->data);
      if (!actor) {
        LOG_ERROR(logger, "CoroutineServiceHandler::ProcessTimer Coroutine is gone:"
            << current->data);
        continue;
      }
      coro::Schedule(actor);
    }
    current = current->next;
  }
  timer_.DestroyTimerNode(timer_header);
}

void CoroutineServiceHandler::ProcessMessage(StageData<ServiceHandler>* data) {
  ProtocolMessage* message = NULL;
  int wait_time = 1;

  while (true) {
    if (!data->queue->Pop(&message, wait_time)) {
      break;
    }

    wait_time = -1;

    if (UNLIKELY(message->type_id == ade::MessageType::kTimerMessage)) {
      TimerMessage* timer_message = static_cast<TimerMessage*>(message);
      coro::SpawnAndSchedule<CoroTask>(std::move(timer_message->proc));
      MessageFactory::Destroy(message);
      continue;
    }

    // if handler is unspecified then this is a new request
    CoroActor* actor = NULL;
    if (message->handler_id == Handler::kUnspecifiedId) {
      Service* service = message->GetPayloadAs<Service*>();
      actor = coro::Spawn<CoroService>(service);
    } else {
      actor = coro::GetCoroutine<CoroActor>(message->GetPayloadAs<uint64_t>());
      if (UNLIKELY(!actor)) {
        LOG_DEBUG(logger, "CoroutineServiceHandler::ProcessMessage Coroutine is gone" 
            << ", coro_id: " << message->GetPayloadAs<uint64_t>()
            << ", message:" << *message);
      } 
    }

    if (LIKELY(actor && actor->SendMail(message))) {
      coro::Schedule(actor);
    } else {
      MessageFactory::Destroy(message);
    }
  }  
}

} //namespace ade
