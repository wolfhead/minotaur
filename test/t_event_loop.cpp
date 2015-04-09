#define BOOST_AUTO_TEST_MAIN

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include <sys/epoll.h>
#include <boost/test/unit_test.hpp>
#include <event/event_loop.h>
#include <event/event_loop_stage.h>
#include <net/socket_op.h>
#include <net/acceptor.h>
#include <net/client_channel.h>
#include <net/io_handler.h>
#include <common/system_error.h>
#include <io_service.h>

#include <service/service_handler.h>
#include <net/io_descriptor_factory.h>
#include "unittest_logger.h"

using namespace minotaur;
using namespace minotaur::event;
using namespace minotaur::unittest;

static minotaur::unittest::UnittestLogger logger_config(log4cplus::TRACE_LOG_LEVEL);
LOGGER_STATIC_DECL_IMPL(logger, "root");
LOGGER_STATIC_DECL_IMPL(g_logger, "root");

BOOST_AUTO_TEST_SUITE(TestEventLoop);

class TestServiceHandler : public ServiceHandler {
 public:
  TestServiceHandler(IOService* io_service)
      : ServiceHandler(io_service) {
  }

  virtual void OnHttpRequestMessage(HttpMessage* message) {
    GetIOService()->GetIOStage()->Send(
        EventMessage(
            minotaur::MessageType::kIOMessageEvent, 
            message->descriptor_id,
            (uint64_t)message));
  } 

  virtual void OnRapidRequestMessage(RapidMessage* message) {
    LOG_INFO(g_logger, "OnRapidRequestMessage");

    message->body = "response";

    GetIOService()->GetIOStage()->Send(
        EventMessage(
            minotaur::MessageType::kIOMessageEvent, 
            message->descriptor_id,
            (uint64_t)message));
  } 

  virtual void OnRapidResponseMessage(RapidMessage* message) {
    LOG_INFO(g_logger, "OnRapidResponseMessage");
  }

  virtual void OnLineRequestMessage(LineMessage* message) {
    LOG_INFO(g_logger, "OnLineRequestMessage:" << message->body);

    message->body = "response";

    GetIOService()->GetIOStage()->Send(
        EventMessage(
            minotaur::MessageType::kIOMessageEvent, 
            message->descriptor_id,
            (uint64_t)message));
  } 

  virtual void OnLineResponseMessage(LineMessage* message) {
    LOG_INFO(g_logger, "OnLineResponseMessage");
  }

  virtual TestServiceHandler* Clone() {
    return new TestServiceHandler(GetIOService());
  }
};

BOOST_AUTO_TEST_CASE(TestEventLoopStage) {

  IOService io_service;
  IOServiceConfig config;
  config.fd_count = 65535;
  config.event_loop_worker = 2;
  config.io_worker = 4;
  config.io_queue_size = 1024 * 1024;
  config.service_worker = 2;
  config.service_queue_size = 1024 * 1024;
  config.service_handler_prototype = 
    new TestServiceHandler(&io_service);

  int ret = io_service.Init(config);
  BOOST_CHECK_EQUAL(ret, 0);

  io_service.HandleSignal();
  ret = io_service.Start();
  BOOST_CHECK_EQUAL(ret, 0);

  Acceptor* http_acceptor = IODescriptorFactory::Instance()
    .CreateAcceptor(&io_service, "http://0.0.0.0:6600");
  ret = http_acceptor->Start();
  BOOST_CHECK_EQUAL(0, ret);
  
  Acceptor* line_acceptor = IODescriptorFactory::Instance()
    .CreateAcceptor(&io_service, "line://0.0.0.0:6601");
  ret = line_acceptor->Start();
  BOOST_CHECK_EQUAL(0, ret);

  Acceptor* rapid_acceptor = IODescriptorFactory::Instance()
    .CreateAcceptor(&io_service, "rapid://0.0.0.0:6602");

  ret = rapid_acceptor->Start();
  BOOST_CHECK_EQUAL(0, ret);

  ClientChannel* rapid_connector = IODescriptorFactory::Instance()
    .CreateClientChannel(&io_service, "rapid://localhost:6604", 2000);
  ret = rapid_connector->Start();

  ClientChannel* line_connector = IODescriptorFactory::Instance()
    .CreateClientChannel(&io_service, "line://localhost:6601", 2000);
  ret = line_connector->Start();

  BOOST_CHECK_EQUAL(0, ret);

  int i = 0;
  while (i--) {
    sleep(2);

    RapidMessage* message = MessageFactory::Allocate<RapidMessage>();

    io_service.GetIOStage()->Send(
        EventMessage(
            minotaur::MessageType::kIOMessageEvent, 
            rapid_connector->GetDescriptorId(),
            (uint64_t)message));

    sleep(2);
  }

  ret = io_service.Run();
  BOOST_CHECK_EQUAL(0, ret);
  
  
  IODescriptorFactory::Instance().Destroy(http_acceptor);
  IODescriptorFactory::Instance().Destroy(line_acceptor);
  IODescriptorFactory::Instance().Destroy(rapid_acceptor);
  IODescriptorFactory::Instance().Destroy(line_connector);
  IODescriptorFactory::Instance().Destroy(rapid_connector);
  
}

BOOST_AUTO_TEST_SUITE_END()
