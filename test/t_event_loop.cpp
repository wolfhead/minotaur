#define BOOST_AUTO_TEST_MAIN

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include <boost/test/unit_test.hpp>
#include <event/event_loop.h>
#include <event/event_loop_stage.h>
#include <net/socket_op.h>
#include <net/acceptor.h>
#include <common/system_error.h>
#include <io_service.h>
#include "unittest_logger.h"

using namespace minotaur;
using namespace minotaur::event;
using namespace minotaur::unittest;

static minotaur::unittest::UnittestLogger logger_config;
LOGGER_STATIC_DECL_IMPL(logger, "root");

BOOST_AUTO_TEST_SUITE(TestEventLoop);

void FileEventProc(EventLoop *eventLoop, int fd, void *clientData, uint32_t mask) {
  std::cout << "EventProc ->" << std::endl;
  char buffer[1024] = {0};
  std::string result;

  if (mask & EventType::EV_READ) {
    while (1) {
      int got = read(fd, buffer, 1024);
      if (got <= 0) {
        std::cerr << SystemError::FormatMessage() << std::endl;
        break;
      }

      result.append(buffer, got);
    }
    
    std::cout << result << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestCompile) {
  EventLoop el;
  int ret = 0;

  ret = el.Init(65535);
  BOOST_CHECK_EQUAL(ret, 0);

  ret = SocketOperation::SetNonBlocking(0);
  BOOST_CHECK_EQUAL(0, ret);

  int i = 10;
  ret = el.AddEvent(0, EventType::EV_READ, &FileEventProc, NULL);
  BOOST_CHECK_EQUAL(0, ret);

  while (i--) {
    ret = el.ProcessEvent(1000);
  }
}

BOOST_AUTO_TEST_CASE(TestEventLoopStage) {
  EventLoopStage stage(4, 65535);
  int ret = stage.Start();
  BOOST_CHECK_EQUAL(0, ret);

  IOService io_service;
  io_service.SetEventLoopStage(&stage);

  Acceptor acceptor(&io_service);
  ret = acceptor.Accept("0.0.0.0", 4433);
  BOOST_CHECK_EQUAL(0, ret);

  sleep(100);

  ret = stage.Stop();
  BOOST_CHECK_EQUAL(0, ret);
}

BOOST_AUTO_TEST_SUITE_END()
