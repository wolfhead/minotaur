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
#include <common/system_error.h>
#include <io_service.h>
#include <net/io_descriptor_factory.h>
#include "unittest_logger.h"

using namespace minotaur;
using namespace minotaur::event;
using namespace minotaur::unittest;

static minotaur::unittest::UnittestLogger logger_config(log4cplus::WARN_LOG_LEVEL);
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
  config.event_loop_worker_ = 1;
  config.io_worker_ = 3;
  config.io_queue_size_ = 1024 * 1024;

  IOService io_service(config);
  int ret = io_service.Start();
  BOOST_CHECK_EQUAL(ret, 0);

  Acceptor* acceptor = IODescriptorFactory::Instance()
    .CreateAcceptor(
        &io_service, "0.0.0.0", 
        4433, ProtocolType::kHttpProtocol);
  ret = acceptor->Start();
  BOOST_CHECK_EQUAL(0, ret);

  sleep(300);

  ret = io_service.Stop();
  BOOST_CHECK_EQUAL(0, ret);
}

BOOST_AUTO_TEST_SUITE_END()
