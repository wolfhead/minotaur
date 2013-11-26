#define BOOST_AUTO_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <event/event_loop.h>
#define private public
#include <event/timer_loop.h>
#undef private

using namespace minotaur::event;

BOOST_AUTO_TEST_SUITE(TestTimerWheel);

BOOST_AUTO_TEST_CASE(Test) {