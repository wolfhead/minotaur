#define BOOST_AUTO_TEST_MAIN

#include <stdio.h>
#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#define private public
#include <lockfree/lf_static_list_pool.h>
#undef private

using namespace ade::lockfree;

BOOST_AUTO_TEST_SUITE(TestLFStaticListPool);

struct Test {
  int a;
  int b;
};


BOOST_AUTO_TEST_CASE(TestConstructDefault) {
  LFStaticListPool<int> int_list(1024);
  LFStaticListPool<Test> struct_list(1024);
}

BOOST_AUTO_TEST_CASE(TestAlloc) {
  LFStaticListPool<Test> struct_list(1024);

  int i = 1024;
  Test* p;
  bool ret;
  while (i--) {
    ret = struct_list.Alloc(&p);
    BOOST_CHECK_EQUAL(ret, true);
  }

  i = 1024;
  while (i--) {
    ret = struct_list.Alloc(&p);
    BOOST_CHECK_EQUAL(ret, false);
  }
}

BOOST_AUTO_TEST_CASE(TestFree) {
  LFStaticListPool<Test> struct_list(1024);

  int i = 1024;
  Test* p;
  bool ret;

  std::vector<Test*> vec;


  while (i--) {
    ret = struct_list.Alloc(&p);
    BOOST_CHECK_EQUAL(ret, true);

    vec.push_back(p);
  }

  for (size_t i = 0; i != vec.size(); ++i) {
    ret = struct_list.Free(p);
    BOOST_CHECK_EQUAL(ret, true);
  }
}

BOOST_AUTO_TEST_CASE(TestAt) {
  LFStaticListPool<Test> struct_list(1024);
  Test* p = NULL;
  bool ret;
  std::vector<Test*> vec;


  for (size_t i = 0; i != 1024; ++i) {
    ret = struct_list.At(i, &p);
    BOOST_CHECK_EQUAL(ret, true);

    p->a = i;
  }

  for (size_t i = 0; i != 1024; ++i) {
    ret = struct_list.At(i, &p);
    BOOST_CHECK_EQUAL(p->a, i);
  }
}

namespace {

void LCThreadProc(LFStaticListPool<Test>* pool, bool report) {
  if (report) {
    printf("ThreadProc start\n");
  }

  std::vector<Test*> vec;
  Test* p;

  int alloc_fail = 0;
  int free_fail = 0;

  int i = 1024 * 128;
  while (i--) {
    if (!pool->Alloc(&p)) {
      ++alloc_fail;
    } else {
      vec.push_back(p);
    }

    if (i % 10000 == 0 && report) {
      printf("Alloc %d item\n", i);
    }
  }

  for (size_t i = 0; i != vec.size(); ++i) {
    if (!pool->Free(vec[i])) {
      ++free_fail;
    }

    if (i % 10000 == 0 && report) {
      printf("Free %lu item\n", i);
    }
  }

  if (report) {
    printf("alloc fail: %d\nfree fail: %d\n", alloc_fail, free_fail);
  }
}
}

BOOST_AUTO_TEST_CASE(TestBench) {
  LFStaticListPool<Test> struct_list(1024 * 1024);

  std::vector<boost::thread*> threads;

  for (size_t i = 0; i != 10; ++i) {
    boost::thread* t = new boost::thread(boost::bind(&LCThreadProc, &struct_list, true));
    threads.push_back(t);
  }

  for (std::vector<boost::thread*>::iterator it = threads.begin();
       it != threads.end();
       ++it) {
    (*it)->join();
    delete *it;
  }
}

BOOST_AUTO_TEST_SUITE_END()
