#define BOOST_AUTO_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <message_queue.h>
#include <common/logger.h>
#include "unittest_logger.h"

using namespace minotaur;
using namespace minotaur::unittest;

static minotaur::unittest::UnittestLogger logger_config;
LOGGER_STATIC_DECL_IMPL(logger, "root");

BOOST_AUTO_TEST_SUITE(TestMessageQueue);

typedef MessageQueue<volatile int> QueueType;

namespace {

static bool g_running = true;

void Consume(QueueType* queue, int target, bool check) {
  int i = 0;
  bool ret = false;
  int tmp = 0;
  while (i != target) {
    ret = queue->Consume(&tmp, 5000);
    if (!ret) {
      continue;
    }
    if (check && (!ret || i != tmp)) {
      LOG_ERROR(logger, "Echo fail"
          << ", ret:" << ret 
          << ", tmp:" << tmp
          << ", i:" << i);
    } else {
      ++i;
    }
  }

  LOG_TRACE(logger, "Consumer count:" << i);
}

void Produce(QueueType* queue, int target) {
  int i = 0;
  for (i = 0; i != target; ++i) {
    while (!queue->Produce(i)) {
    }
  }

  LOG_TRACE(logger, "Produce count:" << i);
}

} //namespace 
/*
BOOST_AUTO_TEST_CASE(testSS) {
  MessageQueue<int> queue(1024 * 1024);
  auto start = boost::posix_time::second_clock::local_time();
  
  boost::thread t(boost::bind(&Consume, &queue, 10000000, true));
  for (int i = 0; i != 10000000; ++i) {
    while (!queue.Produce(i)) {
    }
  }

  g_running = false;
  t.join();

  auto end  = boost::posix_time::second_clock::local_time();
  auto diff = end - start;
  
  LOG_TRACE(logger, "testSS finish:" << diff.total_milliseconds(););
}
*/

BOOST_AUTO_TEST_CASE(testMM) {
  QueueType queue(1024 * 1024);
  size_t producer_count = 3;
  size_t producer_target = 10000000;

  auto start = boost::posix_time::microsec_clock::local_time();
  
  std::vector<boost::thread*> producer;
  for (size_t i = 0; i != producer_count; ++i) {
    producer.push_back(new boost::thread(&Produce, &queue, producer_target));
  }
  
  boost::thread t(boost::bind(
        &Consume, 
        &queue, 
        producer_target * producer_count, 
        producer_count == 1 ? true : false));

  g_running = false;
  t.join();
  LOG_TRACE(logger, "consumer finish:");


  auto end  = boost::posix_time::microsec_clock::local_time();
  auto diff = end - start;
  
  for (size_t i = 0; i != producer_count; ++i) {
    producer[i]->join();
    delete producer[i];  
  }

  LOG_TRACE(logger, "testMM finish:" << diff.total_milliseconds());
}

BOOST_AUTO_TEST_SUITE_END()
