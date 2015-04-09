/**
 * @file coroutine_service_handler.cpp
 * @author Wolfhead
 */
#include "coroutine_service_handler.h"
#include <sys/prctl.h>
#include "../coroutine/coro_all.h"
#include "../service/service.h"

namespace minotaur {

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
      CoroTask* task = coro::GetCoroutine<CoroTask>(current->data);
      if (!task) {
        LOG_ERROR(logger, "CoroutineServiceHandler::ProcessTimer Coroutine is gone:"
            << current->data);
      }
      coro::Schedule(task);
    }
    current = current->next;
  }
}

void CoroutineServiceHandler::ProcessMessage(StageData<ServiceHandler>* data) {
  ProtocolMessage* message = NULL;
  int wait_time = 1;

  while (true) {
    if (!data->pri_queue->Pop(&message)) {
      if (!data->queue->Pop(&message, wait_time)) {
        break;
      }
    }

    wait_time = -1;

    // if handler is unspecified then this is a new request
    CoroActor* actor = NULL;
    if (message->handler_id == Handler::kUnspecifiedId) {
      Service* service = message->GetPayloadAs<Service*>();
      actor = coro::Spawn<CoroService>(service);
      actor->SendMail(message);
    } else {
      actor = coro::GetCoroutine<CoroActor>(message->GetPayloadAs<uint64_t>());
      if (!actor) {
        LOG_ERROR(logger, "CoroutineServiceHandler::ProcessMessage Coroutine is gone:" 
            << message->GetPayloadAs<uint64_t>());
        continue;
      } 
      actor->SendMail(message);
    }
    //LOG_TRACE(logger, "test");
    LOG_WARN(logger, "schedule:" << actor->GetCoroutineId() << ", message:" << *message);
    //coro::Transfer(actor);
    coro::Schedule(actor);
  }  
}

} //namespace minotaur
