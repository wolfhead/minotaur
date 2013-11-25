#define BOOST_AUTO_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#define private public
#include <lock_free_ring.hpp>
#undef private
#include <allspark/fifo.h>

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

  result = ring.Pop(value);
  BOOST_CHECK_EQUAL(true, result);
  BOOST_CHECK_EQUAL(1, value);
  BOOST_CHECK_EQUAL(2, ring.Size());

  result = ring.Pop(value);
  BOOST_CHECK_EQUAL(true, result);
  BOOST_CHECK_EQUAL(2, value);
  BOOST_CHECK_EQUAL(1, ring.Size());

  result = ring.Pop(value);
  BOOST_CHECK_EQUAL(true, result);
  BOOST_CHECK_EQUAL(3, value);
  BOOST_CHECK_EQUAL(0, ring.Size());

  result = ring.Pop(value);
  BOOST_CHECK_EQUAL(false, result);
  BOOST_CHECK_EQUAL(0, ring.Size());

  for (int i = 0; i != 1023; ++i) {
    result = ring.Push(i);
    BOOST_CHECK_EQUAL(true, result);
    BOOST_CHECK_EQUAL(1 + i, ring.Size());
  }

  for (int i = 0; i != 1023; ++i) {
    result = ring.Pop(value);
    BOOST_CHECK_EQUAL(true, result);
    BOOST_CHECK_EQUAL(i, value);
    BOOST_CHECK_EQUAL(1023 - 1 - i, ring.Size());
  }

}

static bool push_finish = false;

void ProducerProcLockFree(zmt::LockFreeRing<int>* ring) {
  for (int i = 0; i != 1000000; ++i) {
    if (!ring->Push(i)) {
      printf("push fail:%d\n", i);
    }
  }
}

void ConsumerProcLockFree(zmt::LockFreeRing<int>* ring) {
  int value;
  while (true) {
    if (!ring->Pop(value)) {
      if (!push_finish) {
        sched_yield();
      } else {
        break;
      }
    }
  }
}

void ProducerProcFifo(zmt::FifoQueue<int>* ring) {
  for (int i = 0; i != 1000000; ++i) {
    if (!ring->push_back(0.5, i)) {
      printf("push fail:%d\n", i);
    }
  }
}

void ConsumerProcFifo(zmt::FifoQueue<int>* ring) {
  int value;
  while (true) {
    if (!ring->pop_front(0.5, &value) && push_finish) {
      break;
    }
  }
}

BOOST_AUTO_TEST_CASE(TestThreadingLockFree) {
  zmt::LockFreeRing<int> ring(10240000);

  std::vector<boost::thread*> producer_threads;
  std::vector<boost::thread*> consumer_threads;

  for (size_t i = 0; i != 10; ++i) {
    boost::thread* t = new boost::thread(boost::bind(ConsumerProcLockFree, &ring));
    consumer_threads.push_back(t);
  }

  for (size_t i = 0; i != 10; ++i) {
    boost::thread* t = new boost::thread(boost::bind(ProducerProcLockFree, &ring));
    producer_threads.push_back(t);
  }

  for (std::vector<boost::thread*>::iterator it = producer_threads.begin();
       it != producer_threads.end();
       ++it) {
    (*it)->join();
  }

  std::cout << "producer finish" << std::endl;
  push_finish = true;

  for (std::vector<boost::thread*>::iterator it = consumer_threads.begin();
       it != consumer_threads.end();
       ++it) {
    (*it)->join();
  }
}


BOOST_AUTO_TEST_CASE(TestThreadingFifo) {
  zmt::FifoQueue<int> ring(10240000);

  std::vector<boost::thread*> producer_threads;
  std::vector<boost::thread*> consumer_threads;

  for (size_t i = 0; i != 10; ++i) {
    boost::thread* t = new boost::thread(boost::bind(ConsumerProcFifo, &ring));
    consumer_threads.push_back(t);
  }

  for (size_t i = 0; i != 10; ++i) {
    boost::thread* t = new boost::thread(boost::bind(ProducerProcFifo, &ring));
    producer_threads.push_back(t);
  }

  for (std::vector<boost::thread*>::iterator it = producer_threads.begin();
       it != producer_threads.end();
       ++it) {
    (*it)->join();
  }

  std::cout << "producer finish" << std::endl;
  push_finish = true;

  for (std::vector<boost::thread*>::iterator it = consumer_threads.begin();
       it != consumer_threads.end();
       ++it) {
    (*it)->join();
  }
}


BOOST_AUTO_TEST_SUITE_END()