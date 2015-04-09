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
#include <application/generic_application.h>
#include <application/config_manager.h>
#include <client/client_manager.h>

using namespace minotaur;
LOGGER_STATIC_DECL_IMPL(logger, "root");

int main(int argc, char* argv[]) {
  GenericApplication<ConfigManager, CoroutineServiceHandler> app;
  return 
    app
      .SetOnStart([&](){
        return app.RegisterService("http_echo_handler", [](ProtocolMessage* message){
          coro::Send(message);
        });
      })
      .Run(argc, argv);
}
