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
#include <net/connector.h>
#include <net/io_handler.h>
#include <common/system_error.h>
#include <io_service.h>

#include <service/service_handler.h>
#include <net/io_descriptor_factory.h>
#include "unittest_logger.h"

using namespace minotaur;
using namespace minotaur::event;
using namespace minotaur::unittest;

static minotaur::unittest::UnittestLogger logger_config(log4cplus::INFO_LOG_LEVEL);
LOGGER_STATIC_DECL_IMPL(logger, "root");

BOOST_AUTO_TEST_SUITE(TestEventLoop);

void FileEventProc(EventLoop *eventLoop, int fd, void *clientData, uint32_t mask) {
  std::cout << "EventProc ->" << std::endl;
  char buffer[1024] = {0};
  std::string result;

  return;

  if (mask & EventType::EV_READ) {
    //while (1) {
      int got = read(fd, buffer, 1);
      if (got <= 0) {
        std::cerr << SystemError::FormatMessage() << std::endl;
        //break;
      }

      result.append(buffer, got);
    //}
    
    std::cout << result << std::endl;
  } 

  if (mask & EventType::EV_WRITE) {
    std::cout << "EV_WRITE" << std::endl;
  }
}

class TestServiceHandler : public ServiceHandlerBase {
 public:
  TestServiceHandler(IOService* io_service, StageType* stage)
      : ServiceHandlerBase(io_service, stage) {
  }

  void OnHttpProtocolMessage(
      HttpProtocolMessage* message) {
    GetIOService()->GetIOStage()->Send(
        EventMessage(
            minotaur::MessageType::kIOMessageEvent, 
            message->descriptor_id,
            (uint64_t)message));
  } 
};


BOOST_AUTO_TEST_CASE(TestCompile) {

  SocketOperation::IgnoreSigPipe();

  EventLoop el;
  int ret = 0;

  ret = el.Init(65535);
  BOOST_CHECK_EQUAL(ret, 0);

  ret = SocketOperation::SetNonBlocking(0);
  BOOST_CHECK_EQUAL(0, ret);

  int i = 10;

  int fd[2];
  socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
  SocketOperation::SetNonBlocking(fd[0]);
  SocketOperation::SetNonBlocking(fd[1]);


  struct epoll_event ee;
  struct epoll_event events[1024];
  int epoll_fd = epoll_create(1024);

  ee.events = EPOLLIN | EPOLLET;
  ee.data.fd = fd[0];
  ee.data.u64 = 0;
  epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd[0], &ee);

  while (i--) {
    SocketOperation::Send(fd[1], "1234567890", 10);
    int ret = epoll_wait(epoll_fd, events, 1024, 1000);

    for (int i = 0; i != ret; ++i) {
      std::cout << "fired" << std::endl;
    }

  }
}

BOOST_AUTO_TEST_CASE(TestEventLoopStage) {

  SocketOperation::IgnoreSigPipe();

  IOServiceConfig config;
  config.fd_count = 65535;
  config.event_loop_worker = 1;
  config.io_worker = 2;
  config.io_queue_size = 1024 * 1024;
  config.service_worker = 1;
  config.service_queue_size = 1024 * 1024;
  config.service_handler_factory = 
    new GenericServiceHandlerFactory<TestServiceHandler>();

  IOService io_service(config);
  
  io_service.HandleSignal();
  int ret = io_service.Start();
  BOOST_CHECK_EQUAL(ret, 0);

  Acceptor* http_acceptor = IODescriptorFactory::Instance()
    .CreateAcceptor(
        &io_service, "0.0.0.0", 
        6600, ProtocolType::kHttpProtocol);
  ret = http_acceptor->Start();
  BOOST_CHECK_EQUAL(0, ret);
  
  Acceptor* line_acceptor = IODescriptorFactory::Instance()
    .CreateAcceptor(
        &io_service, "0.0.0.0", 
        6601, ProtocolType::kLineProtocol);
  ret = line_acceptor->Start();
  BOOST_CHECK_EQUAL(0, ret);

  Acceptor* rapid_acceptor = IODescriptorFactory::Instance()
    .CreateAcceptor(
        &io_service, "0.0.0.0", 
        6602, ProtocolType::kRapidProtocol);
  ret = rapid_acceptor->Start();
  BOOST_CHECK_EQUAL(0, ret);

  Connector* rapid_connector = IODescriptorFactory::Instance()
    .CreateConnector(
        &io_service, "127.0.0.1",
        6603, ProtocolType::kRapidProtocol);
  ret = rapid_connector->Start();
  BOOST_CHECK_EQUAL(0, ret);

  ret = io_service.Run();
  BOOST_CHECK_EQUAL(0, ret);
}

BOOST_AUTO_TEST_SUITE_END()
