/**
 * @file http_echo_server.cpp
 * @author Wolfhead
 */
#include <io_service.h>
#include <service/service.h>
#include <service/coroutine_service_handler.h>
#include <coroutine/coro_all.h>
#include <net/io_descriptor_factory.h>
#include <net/acceptor.h>
#include "../test/unittest_logger.h"

using namespace minotaur;
using namespace minotaur::unittest;

static minotaur::unittest::UnittestLogger logger_config(log4cplus::INFO_LOG_LEVEL);
LOGGER_STATIC_DECL_IMPL(logger, "root");

int main(int argc, char* argv[]) {
  ServiceManager::Instance()->RegisterService("default", CreateService([](ProtocolMessage* message){
        coro::Send(EventMessage(
            minotaur::MessageType::kIOMessageEvent,
            message->descriptor_id,
            (uint64_t)message));
        }));

  IOService io_service;
  IOServiceConfig config;
  config.fd_count = 65535;
  config.event_loop_worker = 4;
  config.io_worker = 16;
  config.io_queue_size = 1024 * 1024;
  config.service_worker = 4;
  config.service_queue_size = 1024 * 1024;
  config.service_handler_prototype = 
    new CoroutineServiceHandler(&io_service);

  io_service.HandleSignal();

  int ret = io_service.Init(config);
  if (ret != 0) {
    return -1;
  }

  ret = io_service.Start();
  if (ret != 0) {
    return -1;
  }

  Acceptor* http_acceptor = IODescriptorFactory::Instance()
    .CreateAcceptor(&io_service, "http://0.0.0.0:6600", "default");
  http_acceptor->Start();

  io_service.Run();
  IODescriptorFactory::Instance().Destroy(http_acceptor);

  return 0;
}
