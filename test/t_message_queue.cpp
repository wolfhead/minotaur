#define BOOST_AUTO_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <queue/message_queue.h>
#include <common/logger.h>
#include <common/time_util.h>
#include "unittest_logger.h"

using namespace ade;
using namespace ade::unittest;

static ade::unittest::UnittestLogger logger_config;
LOGGER_STATIC_DECL_IMPL(logger, "root");

BOOST_AUTO_TEST_SUITE(TestMessageQueue);

typedef MessageQueue<uint64_t> QueueType;

namespace {

void Consume(QueueType* queue, boost::atomic<uint64_t>* target, bool check) {
  uint64_t i = 0;
  bool ret = false;
  uint64_t tmp = 0;
  uint64_t loop = true;
  while (loop) {
    ret = queue->Consume(&tmp, 100);
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
      loop = target->fetch_sub(1) > 1;
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

BOOST_AUTO_TEST_CASE(testMM) {
  QueueType queue(1024 * 1024);
  uint64_t producer_count = 1;
  uint64_t consumer_count = 1;
  uint64_t producer_target = 10000000;
  boost::atomic<uint64_t> consumer_target(10000000 * producer_count);

  auto start = boost::posix_time::microsec_clock::local_time();
  
  std::vector<boost::thread*> producer;
  std::vector<boost::thread*> consumer;
  for (uint64_t i = 0; i != producer_count; ++i) {
    producer.push_back(new boost::thread(&Produce, &queue, producer_target));
  }
  for (uint64_t i = 0; i != consumer_count; ++i) {
    consumer.push_back(new boost::thread(
      boost::bind(
        &Consume, 
        &queue, 
        &consumer_target, 
        producer_count == 1 && consumer_count == 1 ? true : false)));
  }
 
  for (uint64_t i = 0; i != producer_count; ++i) {
    producer[i]->join();
    delete producer[i];  
  }
  for (uint64_t i = 0; i != consumer_count; ++i) {
    consumer[i]->join();
    delete consumer[i];  
  }

  auto end  = boost::posix_time::microsec_clock::local_time();
  auto diff = end - start;
  LOG_TRACE(logger, "testMM finish:" << diff.total_milliseconds());
}

namespace {

void ConsumeLatency(QueueType* queue, uint64_t target) {
  uint64_t i = 0;
  bool ret = false;
  uint64_t tmp = 0;

  int count = 0;
  uint64_t sum = 0;

  while (i != target) {
    ret = queue->Consume(&tmp, 100);
    if (!ret) {
      continue;
    }
    sum += (Time::GetMicrosecond() - tmp);
    ++count;
    if (count == 1000000) {
      LOG_TRACE(logger, "avg:" << sum / count);
      count = 0;
    }
    ++i;
  }

  LOG_TRACE(logger, "Consumer count:" << i);
}

void ProduceLatency(QueueType* queue, int target) {
  int i = 0;
  for (i = 0; i != target; ++i) {
    while (!queue->Produce(Time::GetMicrosecond())) {
    }
  }

  LOG_TRACE(logger, "Produce count:" << i);
}

}

BOOST_AUTO_TEST_CASE(testLatency) {
  QueueType queue(1024 * 1024);
  size_t producer_count = 1;
  size_t producer_target = 10000000;

  auto start = boost::posix_time::microsec_clock::local_time();
  
  std::vector<boost::thread*> producer;
  for (size_t i = 0; i != producer_count; ++i) {
    producer.push_back(new boost::thread(&ProduceLatency, &queue, producer_target));
  }
  
  boost::thread t(boost::bind(
        &ConsumeLatency, 
        &queue, 
        producer_target * producer_count));

  t.join();
  LOG_TRACE(logger, "consumer finish:");

  auto end  = boost::posix_time::microsec_clock::local_time();
  auto diff = end - start;
  
  for (size_t i = 0; i != producer_count; ++i) {
    producer[i]->join();
    delete producer[i];  
  }

  LOG_TRACE(logger, "testLatency finish:" << diff.total_milliseconds());
}

BOOST_AUTO_TEST_SUITE_END()
