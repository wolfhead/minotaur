#define BOOST_AUTO_TEST_MAIN

#include <boost/thread.hpp>
#include <boost/test/unit_test.hpp>
#include <lockfree/freelist.hpp>

using namespace minotaur::lockfree;

BOOST_AUTO_TEST_SUITE(TestFreelist);

struct A {
  uint32_t a;
};

struct B: public A {
  B(uint64_t x, uint64_t y)
  {
  }
  uint32_t b;
};

struct TestStruct {
  uint64_t a;
  uint64_t b;
  uint64_t c;
  uint64_t d;
};

BOOST_AUTO_TEST_CASE(TestInt) {
  std::vector<uint64_t*> pointers;
  std::vector<uint64_t> keys;
  freelist<uint64_t> freelist(1024);

  for (int i = 0; i != 2048; ++i) {
    uint64_t key = 0;
    uint64_t* p = freelist.alloc(&key);
    BOOST_CHECK(p != NULL);
    BOOST_CHECK(key != 0);
    *p = key;
    pointers.push_back(p);
    keys.push_back(key);
  }

  for (auto key : keys) {
    uint64_t* p = freelist.get_key(key);
    BOOST_CHECK(p != NULL);
    BOOST_CHECK(*p == key);
  }

  for(auto &p : pointers){
    bool ret = freelist.destroy(p);
    BOOST_CHECK(ret == true);
  }

  for(auto key : keys){
    bool ret = freelist.destroy_key(key);
    BOOST_CHECK(ret == false);
  }
}

BOOST_AUTO_TEST_CASE(TestIntSize) {
  std::vector<uint64_t*> pointers;
  std::vector<uint64_t> keys;
  freelist<uint64_t> freelist(1, 16);

  for (int i = 0; i != 2048; ++i) {
    uint64_t key = 0;
    uint64_t* p = freelist.alloc(&key);
    BOOST_CHECK(p != NULL);
    BOOST_CHECK(key != 0);
    *p = key;
    pointers.push_back(p);
    keys.push_back(key);
  }

  for (auto key : keys) {
    uint64_t* p = freelist.get_key(key);
    BOOST_CHECK(p != NULL);
    BOOST_CHECK(*p == key);
  }

  for(auto &p : pointers){
    bool ret = freelist.destroy(p);
    BOOST_CHECK(ret == true);
  }

  for(auto key : keys){
    bool ret = freelist.destroy_key(key);
    BOOST_CHECK(ret == false);
  }
}


BOOST_AUTO_TEST_CASE(TestClass) {
  std::vector<A*> pointers;
  freelist<A> freelist(1024, 32);

  for (int i = 0; i != 1024; ++i) {
    uint64_t key = 0;
    A* p = freelist.alloc_with<B>(i, 0, &key);
    BOOST_CHECK(p != NULL);
    BOOST_CHECK(key != 0);
    pointers.push_back(p);
  }

  for(auto &p : pointers){
    bool ret = freelist.destroy(p);
    BOOST_CHECK(ret == true);
  }
}

namespace {

bool g_running_ = true;
std::mutex lock;

void RunnerKey(freelist<std::string>* freelist) {
  uint64_t success_count = 0;
  uint64_t fail_count = 0;
  uint64_t key = 0;

  while(g_running_) {
    std::string* p = freelist->alloc(&key);
    if (p && p->empty()) {
      if (!freelist->destroy_key(key)) {
        std::cerr << "dealloc fail key:" << key << std::endl;
      } else {
        ++success_count;
        continue;
      }
    }
    ++fail_count;
  }
  sleep(1);
  std::cout << "success:" << success_count << ",fail:" << fail_count << std::endl;
}

void RunnerNew() {
  uint64_t success_count = 0;
  uint64_t fail_count = 0;

  while(g_running_) {
    std::lock_guard<std::mutex> guard(lock);
    std::string* p = new std::string;
    if (p && p->empty()) {
      delete p;
      ++success_count;
      continue;
    }
    ++fail_count;
  }
  sleep(1);
  std::cout << "success:" << success_count << ",fail:" << fail_count << std::endl;
}

}

BOOST_AUTO_TEST_CASE(TestThreadingKey) {
  std::vector<boost::thread*> threads;

  freelist<std::string> freelist(1024);
  g_running_ = true;

  for (size_t i = 0; i != 4; ++i) {
    boost::thread* t = new boost::thread(boost::bind(RunnerKey, &freelist));
    threads.push_back(t);
  }

  sleep(20);
  g_running_ = false;

  for (size_t i = 0; i != threads.size(); ++i) {
    threads[i]->join();
    delete threads[i];
  }
}


BOOST_AUTO_TEST_CASE(TestThreadingNew) {
  std::vector<boost::thread*> threads;
  g_running_ = true;

  for (size_t i = 0; i != 4; ++i) {
    boost::thread* t = new boost::thread(boost::bind(RunnerNew));
    threads.push_back(t);
  }

  sleep(20);
  g_running_ = false;

  for (size_t i = 0; i != threads.size(); ++i) {
    threads[i]->join();
    delete threads[i];
  }
}

BOOST_AUTO_TEST_CASE(TestCopyStruct) {
  freelist<TestStruct> freelist(1024);

  TestStruct target;
  int i = 100000000;
  while (i--) {
    TestStruct tmp;
    tmp.a = i;
    tmp.b = i;
    tmp.c = i;
    tmp.d = i;
    target = tmp;
  }
}

BOOST_AUTO_TEST_CASE(TestNewStruct) {
  freelist<TestStruct> freelist(1024);

  TestStruct target;
  int i = 100000000;
  while (i--) {
    TestStruct* tmp = new TestStruct;
    tmp->a = i;
    tmp->b = i;
    tmp->c = i;
    tmp->d = i;
    delete tmp;
  }
}


BOOST_AUTO_TEST_SUITE_END()
