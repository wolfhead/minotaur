#define BOOST_AUTO_TEST_MAIN

#include <boost/thread.hpp>
#include <boost/test/unit_test.hpp>
#define private public
#include <event/timer/timer.h>
#undef private

using namespace minotaur;
using namespace minotaur::event;

BOOST_AUTO_TEST_SUITE(TestEventTimer);

BOOST_AUTO_TEST_CASE(TestAddTimer) {
  Timer<uint64_t> timer(5, 1024);

  timer.AddTimer(4500, 1);

  Timer<uint64_t>::NodeType* node = timer.ProcessTimer();
  BOOST_CHECK(node == NULL);

  sleep(1);
  timer.AddTimer(3500, 2);

  sleep(4);
  node = timer.ProcessTimer();
  BOOST_CHECK(node != NULL);
  BOOST_CHECK(node->data == 1);
  BOOST_CHECK(node->next != NULL);
  BOOST_CHECK(node->next->data == 2);

  timer.DestroyTimerNode(node);
}

BOOST_AUTO_TEST_SUITE_END()
