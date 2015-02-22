#define BOOST_AUTO_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <coroutine/coroutine.h>
#include <coroutine/coro_factory.h>
#include <coroutine/coro_bootstrap.h>
#include <common/logger.h>
#include "unittest_logger.h"

using namespace minotaur;
using namespace minotaur::unittest;

static minotaur::unittest::UnittestLogger logger_config;
LOGGER_STATIC_DECL_IMPL(logger, "root");

BOOST_AUTO_TEST_SUITE(TestCoroutine);

BOOST_AUTO_TEST_CASE(testRun) {
  CoroutineFactory factory(2048);

  Coroutine* c1 = factory.CreateCoroutine();

  int i = 500000000;
  while (i--) {
    Coroutine* c2 = factory.CreateCoroutine();
    c1->Transfer(c2);
  } 
}

BOOST_AUTO_TEST_CASE(testBootstrap) {
  CoroutineFactory factory(2048);
  CoroBootstrap bootstrap;

  bootstrap.Start(factory.CreateCoroutine());
}

class DummyCoroutine : public Coroutine {
 public:
  void SetSum(int* sum) {
    sum_ = sum;
  }

 protected:
  void Run() {
    ++(*sum_);
  }

 private:
  int* sum_; 
};

class SpawCoroutine : public Coroutine {
 protected:
  void Run() {
    int i = 100000000;
    int sum = 0;
    while (i--) {
      Coroutine* coro = GetCoroutineFactory()->CreateCoroutineTask([&](){++sum;});
      Coroutine::Current()->Transfer(coro);
    }

  } 
};

BOOST_AUTO_TEST_CASE(testBootstrapWithClass) {
  CoroutineFactory factory(1024);

  CoroBootstrap bootstrap;
  bootstrap.Start(factory.CreateCoroutine<SpawCoroutine>());
}


BOOST_AUTO_TEST_CASE(testBootstrapWithTask) {
  CoroutineFactory factory(1024);

  CoroBootstrap bootstrap;
  int sum = 0;

  bootstrap.Start(factory.CreateCoroutineTask([&](){
        int i = 100000000;
        while (i--) {
          Coroutine* coro = factory.CreateCoroutineTask([&](){++sum;});
          Coroutine::Current()->Transfer(coro);
        }
      }));

  BOOST_CHECK_EQUAL(100000000, sum);
}

BOOST_AUTO_TEST_SUITE_END()

