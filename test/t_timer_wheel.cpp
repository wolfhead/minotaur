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
  BOOST_CHECK_EQUAL(tw.wheel_size_, 1024 * 16);
  BOOST_CHECK_EQUAL(tw.pending_size_, 16);
  BOOST_CHECK_EQUAL(tw.interval_usec_, 1000);
  
  BOOST_CHECK_EQUAL(tw.current_sec_, 0);
  BOOST_CHECK_EQUAL(tw.current_usec_, 0);
  BOOST_CHECK_EQUAL(tw.current_index_, 0);

  BOOST_CHECK_EQUAL(true, tw.wheel_slots_ != NULL);
  BOOST_CHECK_EQUAL(true, tw.pending_slots_ != NULL);

  TimerEvent* event;
  tw.event_pool_->At(1, &event);
  BOOST_CHECK_EQUAL(1, event->id);

  tw.event_pool_->At(65535, &event);
  BOOST_CHECK_EQUAL(65535, event->id);

  bool pop = tw.wheel_slots_[10001].PopEvent(&event);
  BOOST_CHECK_EQUAL(false, pop);
}

namespace {

static bool g_running = true;
struct ClientData {
  int64_t sec;
  int64_t usec;
};

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
  int64_t sec;
  int64_t usec;
  ClientData* data = (ClientData*)client_data;
  TimerWheel::GetCurrentTime(&sec, &usec);
  std::cout << "OnTimer:" << id 
            << ", current_sec:" << sec 
            << ", current_usec:" << usec 
            << ", target_sec:" << data->sec
            << ", target_usec:" << data->usec << std::endl;
}

} //namespace

BOOST_AUTO_TEST_CASE(TestTimer) {
  TimerWheel tw;
  tw.Init(10000, 1000, 65535);
  boost::thread timer_thread(boost::bind(TimerLoop, &tw));

  std::cout << "waiting for input:" << std::endl;

  int64_t timeout_usec;
  while (std::cin >> timeout_usec) {
    ClientData* pdata = new ClientData;
    TimerWheel::GetCurrentTime(&pdata->sec, &pdata->usec);
    TimerWheel::AddMicroSecond(&pdata->sec, &pdata->usec, timeout_usec);

    tw.AddEvent(pdata->sec, pdata->usec, TimerProc, pdata);
  }

  std::cout << "done" << std::endl;

  g_running = false;
  timer_thread.join();
}

BOOST_AUTO_TEST_SUITE_END()

