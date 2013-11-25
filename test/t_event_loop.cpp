#define BOOST_AUTO_TEST_MAIN

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include <boost/test/unit_test.hpp>
#include <event/event_loop.h>


using namespace minotaur::event;

BOOST_AUTO_TEST_SUITE(TestEventLoop);

void FileEventProc(EventLoop *eventLoop, int fd, void *clientData, int mask) {

  std::cout << "EventProc ->" << std::endl;

  char buffer[1024] = {0};
  std::string result;

  if (mask | EventType::EV_READ) {
    while (1) {
      int got = read(fd, buffer, 1024);
      if (got <= 0) {
        std::cerr << strerror(errno) << std::endl;
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

  int flags = fcntl(0, F_GETFL, 0);
  fcntl(0, F_SETFL, flags | O_NONBLOCK);
  ret = el.AddEvent(0, EventType::EV_READ, &FileEventProc, NULL);
  BOOST_CHECK_EQUAL(ret, 0);

  while (true) {
    ret = el.ProcessEvent();
    std::cout << "ProcessEvent: " << ret << std::endl;
  }
}

BOOST_AUTO_TEST_SUITE_END()
