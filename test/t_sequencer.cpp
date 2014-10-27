#define BOOST_AUTO_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#define private public
#include <queue/sequencer.hpp>
#undef private
#include <common/logger.h>
#include <common/time_util.h>
#include "unittest_logger.h"

BOOST_AUTO_TEST_SUITE(TestSequencer);

static minotaur::unittest::UnittestLogger logger_config;
LOGGER_STATIC_DECL_IMPL(logger, "root");

using namespace minotaur;
using namespace minotaur::queue;

//#define CHECK_RESULT 1
typedef minotaur::queue::MPMCQueue<int64_t, ConditionVariableStrategy<0, 256> > Sequencer;

BOOST_AUTO_TEST_CASE(TestRingBufferGetIndex) {
  minotaur::queue::RingBuffer<int> ring(1024);

  uint32_t result;
  result = ring.Index(0);
  BOOST_CHECK_EQUAL(result, 0);

  result = ring.Index(1);
  BOOST_CHECK_EQUAL(result, 1);

  result = ring.Index(1023);
  BOOST_CHECK_EQUAL(result, 1023);

  result = ring.Index(1024);
  BOOST_CHECK_EQUAL(result, 0);

  result = ring.Index(1025);
  BOOST_CHECK_EQUAL(result, 1);
}

BOOST_AUTO_TEST_CASE(TestInit) {
  Sequencer sequencer(1024);
  BOOST_CHECK_EQUAL(0, sequencer.Size());

  bool result = false;
  int64_t value = 0;
  result = sequencer.Push(1);
  BOOST_CHECK_EQUAL(true, result);
  BOOST_CHECK_EQUAL(1, sequencer.Size());

  result = sequencer.Push(2);
  BOOST_CHECK_EQUAL(true, result);
  BOOST_CHECK_EQUAL(2, sequencer.Size());

  result = sequencer.Push(3);
  BOOST_CHECK_EQUAL(true, result);
  BOOST_CHECK_EQUAL(3, sequencer.Size());

  result = sequencer.Pop(&value, 1);
  BOOST_CHECK_EQUAL(true, result);
  BOOST_CHECK_EQUAL(1, value);
  BOOST_CHECK_EQUAL(2, sequencer.Size());

  result = sequencer.Pop(&value, 1);
  BOOST_CHECK_EQUAL(true, result);
  BOOST_CHECK_EQUAL(2, value);
  BOOST_CHECK_EQUAL(1, sequencer.Size());

  result = sequencer.Pop(&value, 1);
  BOOST_CHECK_EQUAL(true, result);
  BOOST_CHECK_EQUAL(3, value);
  BOOST_CHECK_EQUAL(0, sequencer.Size());

  if (!sequencer.WouldBlock()) {
    result = sequencer.Pop(&value, 1);
    BOOST_CHECK_EQUAL(false, result);
    BOOST_CHECK_EQUAL(0, sequencer.Size());
  }

  for (int i = 0; i != 1023; ++i) {
    result = sequencer.Push(i);
    BOOST_CHECK_EQUAL(true, result);
    BOOST_CHECK_EQUAL(1 + i, sequencer.Size());
  }

  for (int i = 0; i != 1023; ++i) {
    result = sequencer.Pop(&value, 1);
    BOOST_CHECK_EQUAL(true, result);
    BOOST_CHECK_EQUAL(i, value);
    BOOST_CHECK_EQUAL(1023 - 1 - i, sequencer.Size());
  }
}

namespace {

static volatile bool g_running = true;

void ProducerProcLockFree(Sequencer* ring, int id, int count) {
  int push_fail = 0;
  int i = 0;
  while (i != count) {
    int64_t value = ((int64_t)id << 32) | i;
    if (!ring->Push(value)) {
      ++push_fail;
      //usleep(0);
    } else {
      ++i;
    }

#ifdef CHECK_RESULT
    if ((i & 0xFFFFF) == 0) {
      LOG_TRACE(logger, "ProducerProcLockFree finish"
          << ", push_fail:" << push_fail
          << ", push_success:" << i);
    }
#endif

  }
  LOG_TRACE(logger, "ProducerProcLockFree finish"
      << ", push_fail:" << push_fail
      << ", push_success:" << i);
}

void ConsumerProcLockFree(Sequencer* ring, std::vector<int64_t>* vec, boost::mutex* mutex) {
  int64_t value = -1;
  int pop_success = 0;
  int pop_fail = 0;
  while (true) {
    if (!ring->Pop(&value, 1)) {
      if (!g_running) {
        break;
      } else {
        continue;
      }
    }

    if (value == -1) {
      ++pop_fail;
      continue;
    }

    ++pop_success;

#ifdef CHECK_RESULT
    if ((pop_success & 0xFFFFF) == 0) {
      LOG_TRACE(logger, "ConsumerProcLockFree finish"
          << "pop success:" << pop_success
          << ", pop fail:" << pop_fail);
    }

    int id = value >> 32 & 0xFFFFFFFF;
    value = value & 0xFFFFFFFF;

    {
      boost::mutex::scoped_lock lock(mutex[id]);
      vec[id].push_back(value);
    }
#endif
    value = -1;
  }

  LOG_TRACE(logger, "ConsumerProcLockFree finish"
      << "pop success:" << pop_success
      << ", pop fail:" << pop_fail);
}

}//namespace 

BOOST_AUTO_TEST_CASE(TestThreadingLockFree) {
  static const int ring_size = 1024*1024;
  static const int push_count = 10000000;
  static const int producer_count = 2;
  static const int consumer_count = 2;

  Sequencer ring(ring_size);

  std::vector<boost::thread*> producer_threads;
  std::vector<boost::thread*> consumer_threads;

  std::vector<int64_t> result_vec[producer_count];
  boost::mutex mutex_vec[producer_count];

  auto start = boost::posix_time::microsec_clock::local_time();

  for (int i = 0; i != producer_count; ++i) {
    boost::thread* t = new boost::thread(boost::bind(ProducerProcLockFree, &ring, i, push_count));
    producer_threads.push_back(t);
  }

  for (int i = 0; i != consumer_count; ++i) {
    boost::thread* t = new boost::thread(boost::bind(ConsumerProcLockFree, &ring, result_vec, mutex_vec));
    consumer_threads.push_back(t);
  }

  for (std::vector<boost::thread*>::iterator it = producer_threads.begin();
       it != producer_threads.end();
       ++it) {
    (*it)->join();
  }

  LOG_TRACE(logger, "Producer finish");

  g_running = false;

  for (std::vector<boost::thread*>::iterator it = consumer_threads.begin();
       it != consumer_threads.end();
       ++it) {
    (*it)->join();
  }

  auto end  = boost::posix_time::microsec_clock::local_time();
  auto diff = end - start;
  LOG_TRACE(logger, "TestThreadingLockFree finish:" << diff.total_milliseconds());

#ifdef CHECK_RESULT
  std::cout << "consumer finish, start verify" << std::endl;

  for (int i = 0; i != producer_count; ++i) {
    BOOST_CHECK_EQUAL(result_vec[i].size(), push_count);
    if ((int)result_vec[i].size() != push_count) {
      std::cout << "size not fit, i:" << i;
      continue;
    }

    std::sort(result_vec[i].begin(), result_vec[i].end());

    for (int j = 0; j != push_count; ++j) {
      BOOST_CHECK_EQUAL(result_vec[i][j], j);
    }
  }

  std::cout << "verify finish" << std::endl;
#endif
}

BOOST_AUTO_TEST_SUITE_END()
