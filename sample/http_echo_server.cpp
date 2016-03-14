/**
 * @file http_echo_server.cpp
 * @author Wolfhead
 */
#include <boost/lexical_cast.hpp>
#include <service/coroutine_service_handler.h>
#include <coroutine/coro_all.h>
#include <net/io_message.h>
#include <application/generic_application.h>
#include <application/config_manager.h>
#include <client/client_manager.h>

using namespace ade;
LOGGER_STATIC_DECL_IMPL(logger, "root");

int main(int argc, char* argv[]) {
  GenericApplication<ConfigManager, CoroutineServiceHandler> app;
  return 
    app
      .SetOnStart([&](){
        Application::Get()->RegisterService("http_echo_handler", [](ProtocolMessage* message)  {
          std::string response(1024, 'a');
          HttpMessage* http_message = (HttpMessage*)message;
          coro::Reply(http_message->AsResponse(200, response));
        });

        return 0;
      })
      .Run(argc, argv);
}
