#define BOOST_AUTO_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>
#include <coroutine/coro_all.h>
#include <common/logger.h>
#include "unittest_logger.h"

using namespace ade;
using namespace ade::unittest;

static ade::unittest::UnittestLogger logger_config;
LOGGER_STATIC_DECL_IMPL(logger, "root");

BOOST_AUTO_TEST_SUITE(TestCoroutine);

static int stub1 = (
    ThreadLocalCorotineFactory::GlobalInit(2048), 
    CoroutineContext::Init(ThreadLocalCorotineFactory::Instance(), NULL, NULL, NULL), 
    0);

BOOST_AUTO_TEST_CASE(testRun) {
  Coroutine* c1 = coro::Spawn<Coroutine>();

  int i = 500000;
  while (i--) {
    Coroutine* c2 = coro::Spawn<Coroutine>();
    c1->Transfer(c2);
  } 
}

BOOST_AUTO_TEST_CASE(testBootstrap) {
  CoroBootstrap bootstrap;
  bootstrap.Start(coro::Spawn<Coroutine>());
}

class DummyCoroutine : public Coroutine {
 public:
  DummyCoroutine(int* sum) {
    SetSum(sum);
  }

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
      Coroutine* coro = coro::Spawn<CoroTask>([&](){++sum;});
      coro::Transfer(coro);
    }

  } 
};

BOOST_AUTO_TEST_CASE(testBootstrapWithClass) {
  CoroBootstrap bootstrap;
  bootstrap.Start(coro::Spawn<SpawCoroutine>());
}

BOOST_AUTO_TEST_CASE(testBootstrapWithTask) {
  CoroBootstrap bootstrap;
  int sum = 0;

  bootstrap.Start(coro::Spawn<CoroTask>([&](){
        int i = 100000000;

        //std::string str = boost::lexical_cast<std::string>(1.1);
        while (i--) {
          Coroutine* coro = coro::Spawn<CoroTask>([&](){++sum;});
          coro::Transfer(coro);
        }
      }));

  BOOST_CHECK_EQUAL(100000000, sum);
}

BOOST_AUTO_TEST_CASE(testCoroScheduler) {
  CoroBootstrap bootstrap;

  CoroScheduler* scheduler = coro::Spawn<CoroScheduler>();
  for (int i = 0; i != 1000; ++i) {
    coro::SpawnAndSchedule<CoroTask>([=](){std::cout << i << std::endl;});
  }

  bootstrap.Start(scheduler);
}

struct Data {
  coro_context boot;
  coro_context tmp;
};

void Test(void* p) {
  Data* data = (Data*)p;
  coro_transfer(&(data->tmp), &(data->boot));
}

void Process() {
  Data data;
  coro_create(&(data.boot), NULL, NULL, NULL, 0);

  uint64_t buffer[1024];

  int i = 100000;
  while (i--) {
    coro_create(&(data.tmp), &Test, &data, buffer, 1024);
    coro_transfer(&(data.boot), &(data.tmp));
    coro_destroy(&(data.tmp));
  }
}

BOOST_AUTO_TEST_CASE(testThread) {
  boost::thread t1(Process);
  boost::thread t2(Process);
  t1.join();
  t2.join();
}

BOOST_AUTO_TEST_CASE(testThread2) {
  auto proc = [](){
    CoroutineContext::Init(ThreadLocalCorotineFactory::Instance(), NULL, NULL, NULL);
    CoroutineFactory factory(1024);
    Coroutine* c1 = factory.CreateCoroutine<Coroutine>();


    int i = 5000000;
    while (i--) {
      Coroutine* c2 = factory.CreateCoroutine<Coroutine>();
      c1->Transfer(c2);
    } 
  };

  boost::thread t1(proc);
  boost::thread t2(proc);
  t1.join();
  t2.join();
}

BOOST_AUTO_TEST_SUITE_END()

