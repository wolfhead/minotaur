#define BOOST_AUTO_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#define private public
#include <static_list_pool.h>
#undef private

using namespace minotaur;

BOOST_AUTO_TEST_SUITE(TestStaticList);

struct Test {
  int a;
  int b;
};


BOOST_AUTO_TEST_CASE(TestConstructDefault) {
  StaticListPool<int> int_list(1024);
  StaticListPool<Test> struct_list(1024);
}

BOOST_AUTO_TEST_CASE(TestAlloc) {
  StaticListPool<Test> struct_list(1024);

  int i = 1024;
  Test* p;
  bool ret;
  while (i--) {
    ret = struct_list.Alloc(&p);
    BOOST_CHECK_EQUAL(ret, true);
  }
}

BOOST_AUTO_TEST_CASE(TestFree) {
  StaticListPool<Test> struct_list(1024);

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
  StaticListPool<Test> struct_list(1024);
  Test* p;
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

void LCThreadProc(List<Test>* pool, bool report) {

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
  }

  for (size_t i = 0; i != vec.size(); ++i) {
    if (!pool->Free(vec[i])) {
      ++free_fail;
    }
  }

  if (report) {
    printf("alloc fail: %d\nfree fail: %d\n", alloc_fail, free_fail);
  }
}
}

BOOST_AUTO_TEST_CASE(TestBench) {
  StaticListPool<Test> struct_list(1024 * 1024);
  List<Test> list(1024 * 1024);

  std::vector<boost::thread*> threads;

  for (size_t i = 0; i != 100; ++i) {
    boost::thread* t = new boost::thread(boost::bind(&LCThreadProc, &list, false));
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
