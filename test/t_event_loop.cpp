#define BOOST_AUTO_TEST_MAIN

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <boost/test/unit_test.hpp>
#include <event/event_loop.h>

using namespace minotaur::event;

BOOST_AUTO_TEST_SUITE(TestEventLoop);

void FileEventProc(EventLoop *eventLoop, int fd, void *clientData, int mask) {

  char buffer[1024] = {0};
  std::string result;

  if (mask | EventType::EV_READ) {
    while (1) {
      int got = read(fd, buffer, 1024);
      if (got <= 0) {
        std::cerr << "fail" << std::endl;
        break;
      }

      result.append(buffer, got);
    }
    
    std::cout << result << std::endl;
  }
}


BOOST_AUTO_TEST_CASE(TestCompile) {
  EventLoop el;
  el.Init(65535);
  el.AddEvent(0, EventType::EV_READ, &FileEventProc, NULL);

  while (true) {
    el.ProcessEvent();
  }
}

BOOST_AUTO_TEST_SUITE_END()
