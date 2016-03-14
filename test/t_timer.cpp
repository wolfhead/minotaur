#define BOOST_AUTO_TEST_MAIN

#include <boost/thread.hpp>
#include <boost/test/unit_test.hpp>
#include <common/logger.h>
#define private public
#include <event/timer_thread.h>
#undef private
#include "unittest_logger.h"


using namespace ade;
using namespace ade::event;

static ade::unittest::UnittestLogger logger_config;
LOGGER_STATIC_DECL_IMPL(logger, "root");

BOOST_AUTO_TEST_SUITE(TestTimer);

static TimerThread* g_thread;

void TimerProc(bool active, void* data) {
  std::cout 
    << "active:" << active 
    << ", data:" << (uint64_t)data 
    << ", current:" << Time::GetMillisecond() 
    << ", miss:" << (int64_t)data - (int64_t)Time::GetMillisecond()
    << std::endl;
  g_thread->StartTimer(1000, TimerProc, (void*)(Time::GetMillisecond() + 1000));
}

BOOST_AUTO_TEST_CASE(TestTimer) {
  TimerThread thread(1, 1000 * 60 * 60);
  int ret = 0;

  ret = thread.Start();
  BOOST_CHECK_EQUAL(ret , 0);

  g_thread = &thread;
  thread.StartTimer(1000, TimerProc, (void*)(Time::GetMillisecond() + 1000));

  sleep(20);
  ret = thread.Stop();
  BOOST_CHECK_EQUAL(ret , 0);
}

BOOST_AUTO_TEST_SUITE_END();
