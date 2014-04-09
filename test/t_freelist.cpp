#define BOOST_AUTO_TEST_MAIN

#include <boost/thread.hpp>
#include <boost/test/unit_test.hpp>
#include <lockfree/freelist.hpp>

using namespace minotaur::lockfree;

BOOST_AUTO_TEST_SUITE(TestTaggedPtr);

struct A {
  int a;
  double b;
};

BOOST_AUTO_TEST_CASE(TestInt) {
  std::vector<int*> pointers;
  fix_sized_freelist<int> freelist(1024);

  for (int i = 0; i != 2048; ++i) {
    int* p = freelist.alloc();
    if (p) {
      *p = i;
      pointers.push_back(p);
    }
  }

  for(auto &p : pointers){
    freelist.dealloc(p);
  }
}

BOOST_AUTO_TEST_CASE(TestStruct) {
  std::vector<A*> pointers;
  fix_sized_freelist<A> freelist(1024);

  for (int i = 0; i != 2048; ++i) {
    A* p = freelist.alloc();
    if (p) {
      p->b = 1.123;
      pointers.push_back(p);
    }
  }

  for(auto &p : pointers){
    freelist.dealloc(p);
  }
}

BOOST_AUTO_TEST_CASE(TestString) {
  std::vector<std::string*> pointers;
  fix_sized_freelist<std::string> freelist(1024);

  for (int i = 0; i != 2048; ++i) {
    std::string* p = freelist.alloc();
    if (p) {
      p->assign("wolfhead is big big shabi");
      pointers.push_back(p);
    }
  }

  for(auto &p : pointers){
    freelist.dealloc(p);
  }
}

namespace {

bool g_running_ = true;
void Runner(fix_sized_freelist<std::string>* freelist) {
  while(g_running_) {
    std::string* p = freelist->alloc();
    if (p) {
      p->assign("wo shi da sha bi");
      freelist->dealloc(p);
    }
  }
}

}

BOOST_AUTO_TEST_CASE(TestThreading) {
  std::vector<boost::thread*> threads;

  fix_sized_freelist<std::string> freelist(65536);

  for (int i = 0; i != 20; ++i) {
    boost::thread* t = new boost::thread(boost::bind(Runner, &freelist));
    threads.push_back(t);
  }

  sleep(20);
  g_running_ = false;

  for (int i = 0; i != threads.size(); ++i) {
    threads[i]->join();
    delete threads[i];
  }
}


BOOST_AUTO_TEST_SUITE_END()
