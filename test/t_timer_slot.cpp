#define BOOST_AUTO_TEST_MAIN

#include <stdio.h>
#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/progress.hpp>
#include <event/timer/timer_slot.h>

using namespace minotaur::event;

BOOST_AUTO_TEST_SUITE(TestTimerSlot);

TimerSlot* CreateTimerSlot() {
  return new CasTimerSlot();
}


BOOST_AUTO_TEST_CASE(TestPushPop) {
  TimerSlot* slot = CreateTimerSlot();
  TimerEvent* ev_1 = new TimerEvent();
  TimerEvent* ev_2 = new TimerEvent();
  bool ret = false;
  ev_1->id = 1;
  ev_2->id = 2;

  slot->PushEvent(ev_1);
  slot->PushEvent(ev_2);

  ev_1 = ev_2 = NULL;

  ret = slot->PopEvent(&ev_1);
  BOOST_CHECK_EQUAL(true, ret);
  BOOST_CHECK_EQUAL(2, ev_1->id);

  ret = slot->PopEvent(&ev_2);
  BOOST_CHECK_EQUAL(true, ret);
  BOOST_CHECK_EQUAL(1, ev_2->id);

  delete ev_1;
  delete ev_2;

  ret = slot->PopEvent(&ev_1);
  BOOST_CHECK_EQUAL(false, ret);

  delete slot;
}

BOOST_AUTO_TEST_CASE(TestPlainTimerSlotIterate) {
  PlainTimerSlot* slot = new PlainTimerSlot();
  TimerEvent* ev;
  ev = new TimerEvent();
  ev->id = 4;
  slot->PushEvent(ev);

  ev = new TimerEvent();
  ev->id = 3;
  slot->PushEvent(ev);

  ev = new TimerEvent();
  ev->id = 2;
  slot->PushEvent(ev);

  ev = new TimerEvent();
  ev->id = 1;
  slot->PushEvent(ev);

  ev = new TimerEvent();
  ev->id = 0;
  slot->PushEvent(ev);

  int i = 0;
  PlainTimerSlot::iterator end = slot->end();
  for (PlainTimerSlot::iterator it = slot->begin(); it != end; ++it, ++i) {
    BOOST_CHECK_EQUAL(i, it->id);
  }

  for (PlainTimerSlot::iterator it = slot->begin(); it != end; ++it, ++i) {
    delete *it;
  }

  delete slot;
}

BOOST_AUTO_TEST_CASE(TestPlainTimerSlotRemove) {
  PlainTimerSlot* slot = new PlainTimerSlot();
  TimerEvent* ev;
  ev = new TimerEvent();
  ev->id = 4;
  slot->PushEvent(ev);

  ev = new TimerEvent();
  ev->id = 3;
  slot->PushEvent(ev);

  ev = new TimerEvent();
  ev->id = 2;
  slot->PushEvent(ev);

  ev = new TimerEvent();
  ev->id = 1;
  slot->PushEvent(ev);

  ev = new TimerEvent();
  ev->id = 0;
  slot->PushEvent(ev);

  PlainTimerSlot::iterator end = slot->end();
  for (PlainTimerSlot::iterator it = slot->begin(); it != end; ++it) {
    if (it->id == 2) {
      it = slot->Remove(it);
    }
  }

  int i = 0;
  for (PlainTimerSlot::iterator it = slot->begin(); it != end; ++it, ++i) {
    BOOST_CHECK_EQUAL(i, it->id);
    if (i == 1) ++i;
    delete *it;
  }

  delete slot;

}

namespace {

static bool g_running = true;

void Pusher(TimerSlot* slot, int id, int count) {
  while (count--) {
    TimerEvent* ev = new TimerEvent();
    ev->when_sec = id;
    ev->when_usec = count;
    slot->PushEvent(ev);
  }
}

void Poper(
    TimerSlot* slot, 
    std::vector<int64_t>* result_vec,
    boost::mutex* mutex_vec) {
  TimerEvent* ev = NULL;
  while (true) {
    if (!slot->PopEvent(&ev)) {
      if (!g_running) {
        break;
      } else {
        continue;
      }
    } else {
      boost::mutex::scoped_lock scope_lock(mutex_vec[ev->when_sec]);
      result_vec[ev->when_sec].push_back(ev->when_usec);
      delete ev;
      ev = NULL;
    }
  }
}

} //namespace

BOOST_AUTO_TEST_CASE(TestPerformance) {
  static const size_t pusher_count = 4;
  static const size_t poper_count = 4; 
  static const size_t item_count = 10000000;

  std::cout << "Start Performance Test" << std::endl;

  TimerSlot* slot = CreateTimerSlot();
  std::vector<int64_t> result_vec[pusher_count];
  boost::mutex mutex_vec[pusher_count];

  std::vector<boost::thread*> pusher_threads;
  std::vector<boost::thread*> poper_threads;

  {
    boost::progress_timer pt;
    for (size_t i = 0; i != pusher_count; ++i) {
      boost::thread* t = new boost::thread(boost::bind(Pusher, slot, i, item_count));
      pusher_threads.push_back(t);
    }

    for (size_t i = 0; i != poper_count; ++i) {
      boost::thread* t = new boost::thread(boost::bind(Poper, slot, result_vec, mutex_vec));
      poper_threads.push_back(t);
    }

    for (std::vector<boost::thread*>::iterator it = pusher_threads.begin();
         it != pusher_threads.end();
         ++it) {
      (*it)->join();
      delete *it;
    }

    std::cout << "pusher finish, wait for poper" << std::endl;
    g_running = false;

    for (std::vector<boost::thread*>::iterator it = poper_threads.begin();
         it != poper_threads.end();
         ++it) {
      (*it)->join();
      delete *it;
    }
  }

  std::cout << "poper finish, start verify" << std::endl;

  for (size_t i = 0; i != pusher_count; ++i) {
    BOOST_CHECK_EQUAL(result_vec[i].size(), item_count);
    std::sort(result_vec[i].begin(), result_vec[i].end());

    for (size_t j = 0; j != result_vec[i].size(); ++j) {
      BOOST_CHECK_EQUAL(j, result_vec[i][j]);
    }
  }

  std::cout << "verify complete" << std::endl;

  delete slot;
}

BOOST_AUTO_TEST_SUITE_END()

