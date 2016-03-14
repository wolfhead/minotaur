/**
 * @file http_echo_server.cpp
 * @author Wolfhead
 */
#include <boost/lexical_cast.hpp>
#include <io_service.h>
#include <service/service.h>
#include <service/coroutine_service_handler.h>
#include <coroutine/coro_all.h>
#include <application/generic_application.h>
#include <application/config_manager.h>
#include <client/client_manager.h>
#include <net/io_redis_message.h>

using namespace ade;
LOGGER_STATIC_DECL_IMPL(logger, "root");

int main(int argc, char* argv[]) {
  GenericApplication<ConfigManager, CoroutineServiceHandler> app;
  return 
    app
      .SetOnStart([&](){
        ClientRouter* client = app.GetClientManager()->GetClientRouter("redis");

        ClientManager::SetDefaultTimeout(50);

        Application::Get()->RegisterService("http_invoke_handler", [=](ProtocolMessage* message){
          HttpMessage* http_message = (HttpMessage*)message;
          RedisRequestMessage* redis_message = MessageFactory::Allocate<RedisRequestMessage>();
          redis_message->Set("789", "456");

          RedisResponseMessage* response = client->SendRecieveHash(redis_message, time(NULL));
          if (!response) {
            LOG_DEBUG(logger, "fail");
            http_message->status_code = 500;
            http_message->body = "fail";
          } else {
            http_message->status_code = 200;
          }
          MessageFactory::Destroy(response);
          coro::Reply(message);
        });

        Application::Get()->RegisterService("client_handler", [=](ProtocolMessage* message){
          HttpMessage* http_message = (HttpMessage*)message;
          coro::Reply(http_message->AsResponse(200, Application::Get()->GetClientManager()->Diagnostic()));
        });

        return 0;
      })
      .Run(argc, argv);
}
