#define BOOST_AUTO_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <stage.h>
#include <common/logger.h>
#include <common/time_util.h>
#include "unittest_logger.h"

using namespace minotaur;
using namespace minotaur::unittest;

static minotaur::unittest::UnittestLogger logger_config;
LOGGER_STATIC_DECL_IMPL(logger, "root");

BOOST_AUTO_TEST_SUITE(TestStage);

class DummyHandler {
 public:
  typedef DummyHandler self;
  typedef minotaur::Stage<self> StageType;
  typedef int MessageType;


  static const bool share_handler = true;
  static const bool share_queue = true;

  static uint32_t HashMessage(const MessageType& message, uint32_t worker_count) {
    return 0;
  } 

  void Handle(const MessageType& message) {
    LOG_TRACE(logger, "message:" << message);
    if (message == 0) {
      LOG_TRACE(logger, "quit stage");
      stage_->Stop();
    }
  }

  void SetStage(StageType* stage) {stage_ = stage;}

 private:
  StageType* stage_;
};

class LatencyHandler {
 public:
  typedef LatencyHandler self;
  typedef minotaur::Stage<self> StageType;
  typedef uint64_t MessageType;

  static const bool share_handler = false;
  static const bool share_queue = false;

  static uint32_t HashMessage(const MessageType& message, uint32_t worker_count) {
    return message % worker_count;
  } 

  LatencyHandler() 
      : sum_latency_(0)
      , sum_count_(0) {
  }

  ~LatencyHandler() {
    if (sum_count_ == 0) {
      LOG_TRACE(logger, "sum_count:" << sum_count_);
    } else {
      LOG_TRACE(logger, "sum_count:" << sum_count_
          << ", sum_latency:" << sum_latency_
          << ", avg_latency:" << (double)sum_latency_ / sum_count_);
    }
  }


  void Handle(const MessageType& message) {
    if (message == 0) {
      LOG_TRACE(logger, "quit stage");
      stage_->Stop();
      return;
    }

    sum_latency_ += (Time::GetMicrosecond() - message);
    ++sum_count_;
  }

  void SetStage(StageType* stage) {stage_ = stage;}

 private:
  StageType* stage_;
  uint64_t sum_latency_;
  uint32_t sum_count_;
};


namespace {

void Input(Stage<DummyHandler>* stage) {
  int i = 1000;
  while (i--) {
    stage->Send(i);
  }
}

void InputLatency(Stage<LatencyHandler>* stage, uint32_t count) {
  while (count--) {
    stage->Send(Time::GetMicrosecond());
  }
  stage->Send(0);
}


}//namespace


BOOST_AUTO_TEST_CASE(testSharedStage) {
  Stage<DummyHandler> test_stage(4, 1024 * 1024);
  int ret = 0;

  ret = test_stage.Start();
  BOOST_CHECK_EQUAL(0, ret);

  boost::thread t(&Input, &test_stage);

  test_stage.Wait();
  t.join();
}

BOOST_AUTO_TEST_CASE(testOwnStage) {
  Stage<LatencyHandler> test_stage(4, 1024 * 1024);
  int ret = 0;

  ret = test_stage.Start();
  BOOST_CHECK_EQUAL(0, ret);

  boost::thread t1(boost::bind(&InputLatency, &test_stage, 10000000));


  test_stage.Wait();
  t1.join();
}



BOOST_AUTO_TEST_SUITE_END()


