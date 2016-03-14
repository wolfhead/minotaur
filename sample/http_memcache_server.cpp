/**
 * @file http_echo_server.cpp
 * @author Wolfhead
 */
#include <boost/lexical_cast.hpp>
#include <io_service.h>
#include <service/service.h>
#include <service/coroutine_service_handler.h>
#include <coroutine/coro_all.h>
#include <net/io_descriptor_factory.h>
#include <net/acceptor.h>
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
        ClientRouter* client = app.GetClientManager()->GetClientRouter("rapid");

        Application::Get()->RegisterService("http_invoke_handler", [=](ProtocolMessage* message){
          HttpMessage* http_message = (HttpMessage*)message;
          LineMessage* rapid_message = MessageFactory::Allocate<LineMessage>("get 123");
          rapid_message = client->SendRecieve(rapid_message);
          if (!rapid_message) {
            LOG_ERROR(logger, "fail");
            http_message->status_code = 500;
            http_message->body = "fail";
          } else {
            http_message->status_code = 200;
            http_message->body = "success";           
            MessageFactory::Destroy(rapid_message);
          }

          coro::Reply(message);
        });

        return 0;
      })
      .Run(argc, argv);
}
