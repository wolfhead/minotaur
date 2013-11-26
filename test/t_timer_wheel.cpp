#define BOOST_AUTO_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <event/event_loop.h>
#define private public
#include <event/timer/timer_wheel.h>
#undef private

using namespace minotaur::event;

BOOST_AUTO_TEST_SUITE(TestTimerWheel);

BOOST_AUTO_TEST_CASE(TestInit) {
  TimerWheel tw;
  int ret;

  ret = tw.Init(10000, 1000, 65536);
  BOOST_CHECK_EQUAL(ret, 0);
  BOOST_CHECK_EQUAL(tw.size_, 1024 * 16);
  BOOST_CHECK_EQUAL(tw.interval_usec_, 1000);
  
  BOOST_CHECK_EQUAL(tw.current_sec_, 0);
  BOOST_CHECK_EQUAL(tw.current_usec_, 0);
  BOOST_CHECK_EQUAL(tw.current_index_, 0);

  BOOST_CHECK_EQUAL(true, tw.wheel_slots_ != NULL);
  BOOST_CHECK_EQUAL(true, tw.pending_slot_ != NULL);

  TimerEvent* event;
  tw.event_pool_->At(1, &event);
  BOOST_CHECK_EQUAL(1, event->id);

  tw.event_pool_->At(65535, &event);
  BOOST_CHECK_EQUAL(65535, event->id);

  bool pop = tw.wheel_slots_[10001].PopEvent(&event);
  BOOST_CHECK_EQUAL(true, pop);
}

namespace {

static bool g_running = true;

void TimerLoop(TimerWheel* tw) {
  EventLoop el;
  el.Init(65536);

  std::cout << "TimerLoop Running" << std::endl;

  while (g_running) {
    el.ProcessEvent(1);
    int ret = tw->ProcessEvent();
    if (ret != 0) {
      std::cout << "TimerWheel.ProcessEvent fail" << std::endl;
    }
  }
}

void TimerProc(TimerWheel* tw, int id, void* client_data) {
  std::cout << "OnTimer:" << id << std::endl;
}

} //namespace

BOOST_AUTO_TEST_CASE(TestTimer) {
  TimerWheel tw;
  tw.Init(10000, 1000, 65535);
  boost::thread timer_thread(boost::bind(TimerLoop, &tw));

  std::cout << "waiting for input:" << std::endl;

  int64_t timeout_usec;
  while (std::cin >> timeout_usec) {
    int64_t sec;
    int64_t usec;
    TimerWheel::GetCurrentTime(&sec, &usec);
    TimerWheel::AddMicroSecond(&sec, &usec, timeout_usec);

    tw.AddEvent(sec, usec, TimerProc, NULL);
  }

  std::cout << "done" << std::endl;

  g_running = false;
  timer_thread.join();
}

BOOST_AUTO_TEST_SUITE_END()

