#define BOOST_AUTO_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#define private public
#include <lock_free_ring.hpp>
#undef private

BOOST_AUTO_TEST_SUITE(TestLockFreeRing);

BOOST_AUTO_TEST_CASE(TestRoundUpSize) {
  zmt::LockFreeRing<int> ring(1024);

  uint32_t result;
  result = ring.RoundUpSize(1024);
  BOOST_CHECK_EQUAL(result, 1024);

  result = ring.RoundUpSize(1025);
  BOOST_CHECK_EQUAL(result, 2048);

  result = ring.RoundUpSize(0);
  BOOST_CHECK_EQUAL(result, 0);

  result = ring.RoundUpSize(0x80000000);
  BOOST_CHECK_EQUAL(result, 0x80000000);

  result = ring.RoundUpSize(0x80000001);
  BOOST_CHECK_EQUAL(result, 0);
}

BOOST_AUTO_TEST_CASE(TestGetIndex) {
  zmt::LockFreeRing<int> ring(1024);

  uint32_t result;
  result = ring.GetIndex(0);
  BOOST_CHECK_EQUAL(result, 0);

  result = ring.GetIndex(1);
  BOOST_CHECK_EQUAL(result, 1);

  result = ring.GetIndex(1023);
  BOOST_CHECK_EQUAL(result, 1023);

  result = ring.GetIndex(1024);
  BOOST_CHECK_EQUAL(result, 0);

  result = ring.GetIndex(1025);
  BOOST_CHECK_EQUAL(result, 1);
}

BOOST_AUTO_TEST_CASE(TestInit) {
  zmt::LockFreeRing<int> ring(1024);
  BOOST_CHECK_EQUAL(0, ring.Size());

  bool result = false;
  int value = 0;
  result = ring.Push(1);
  BOOST_CHECK_EQUAL(true, result);
  BOOST_CHECK_EQUAL(1, ring.Size());

  result = ring.Push(2);
  BOOST_CHECK_EQUAL(true, result);
  BOOST_CHECK_EQUAL(2, ring.Size());

  result = ring.Push(3);
  BOOST_CHECK_EQUAL(true, result);
  BOOST_CHECK_EQUAL(3, ring.Size());

  result = ring.Pop(&value);
  BOOST_CHECK_EQUAL(true, result);
  BOOST_CHECK_EQUAL(1, value);
  BOOST_CHECK_EQUAL(2, ring.Size());

  result = ring.Pop(&value);
  BOOST_CHECK_EQUAL(true, result);
  BOOST_CHECK_EQUAL(2, value);
  BOOST_CHECK_EQUAL(1, ring.Size());

  result = ring.Pop(&value);
  BOOST_CHECK_EQUAL(true, result);
  BOOST_CHECK_EQUAL(3, value);
  BOOST_CHECK_EQUAL(0, ring.Size());

  result = ring.Pop(&value);
  BOOST_CHECK_EQUAL(false, result);
  BOOST_CHECK_EQUAL(0, ring.Size());

  for (int i = 0; i != 1023; ++i) {
    result = ring.Push(i);
    BOOST_CHECK_EQUAL(true, result);
    BOOST_CHECK_EQUAL(1 + i, ring.Size());
  }

  for (int i = 0; i != 1023; ++i) {
    result = ring.Pop(&value);
    BOOST_CHECK_EQUAL(true, result);
    BOOST_CHECK_EQUAL(i, value);
    BOOST_CHECK_EQUAL(1023 - 1 - i, ring.Size());
  }

}

namespace {

static volatile bool g_running = true;

void ProducerProcLockFree(zmt::LockFreeRing<int64_t>* ring, int id, int count) {
  int push_fail = 0;
  int i = 0;
  while (i != count) {
    int64_t value = ((int64_t)id << 32) | i;
    if (!ring->Push(value)) {
      ++push_fail;
      sched_yield();
    } else {
      ++i;
    }
  }
  //std::cout << "ProducerProcLockFree finish, push_fail:" << push_fail << std::endl;
}

void ConsumerProcLockFree(zmt::LockFreeRing<int64_t>* ring, std::vector<int64_t>* vec, boost::mutex* mutex) {
  int64_t value = -1;
  int pop_success = 0;
  int pop_fail = 0;
  while (true) {
    if (!ring->Pop(&value)) {
      if (!g_running) {
        break;
      } else {
        sched_yield();
        continue;
      }
    }

    if (value == -1) {
      ++pop_fail;
      continue;
    }

    ++pop_success;
    int id = value >> 32 & 0xFFFFFFFF;
    value = value & 0xFFFFFFFF;

    {
      boost::mutex::scoped_lock lock(mutex[id]);
      vec[id].push_back(value);
    }

    value = -1;
  }

  //std::cout << "ConsumerProcLockFree finish, pop success:" << pop_success
  //          << ", pop fail:" << pop_fail << std::endl;
}

}//namespace 

BOOST_AUTO_TEST_CASE(TestThreadingLockFree) {
  static const int ring_size = 1024;
  static const int push_count = 1000000;
  static const int producer_count = 2;
  static const int consumer_count = 2;


  zmt::LockFreeRing<int64_t> ring(ring_size);

  std::vector<boost::thread*> producer_threads;
  std::vector<boost::thread*> consumer_threads;

  std::vector<int64_t> result_vec[producer_count];
  boost::mutex mutex_vec[producer_count];

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

  std::cout << "producer finish, wait for consumer" << std::endl;
  g_running = false;

  for (std::vector<boost::thread*>::iterator it = consumer_threads.begin();
       it != consumer_threads.end();
       ++it) {
    (*it)->join();
  }

  std::cout << "consumer finish, start verify" << std::endl;

  for (int i = 0; i != producer_count; ++i) {
    BOOST_CHECK_EQUAL(result_vec[i].size(), push_count);
    std::sort(result_vec[i].begin(), result_vec[i].end());

    for (int j = 0; j != push_count; ++j) {
      BOOST_CHECK_EQUAL(result_vec[i][j], j);
    }
  }
}



BOOST_AUTO_TEST_SUITE_END()