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

  for (int i = 0; i != 1024; ++i) {
    int* p = freelist.alloc();
    BOOST_CHECK(p != NULL);
    *p = i;
    pointers.push_back(p);
  }

  for (int i = 0; i != 1024; ++i) {
    int* p = freelist.alloc();
    BOOST_CHECK(p == NULL);
  }

  for(auto &p : pointers){
    bool ret = freelist.dealloc(p);
    BOOST_CHECK(ret == true);
  }
}

BOOST_AUTO_TEST_CASE(TestIntKey) {
  std::vector<uint64_t> keys;
  fix_sized_freelist<int> freelist(1024);

  for (int i = 0; i != 1024; ++i) {
    uint64_t key = freelist.alloc_key();
    BOOST_CHECK(key != 0);
    *freelist.get_key(key) = i;
    keys.push_back(key);
  }

  for (int i = 0; i != 1024; ++i) {
    uint64_t key = freelist.alloc_key();
    BOOST_CHECK(key == 0);
  }

  for(auto &p : keys){
    bool ret = freelist.dealloc_key(p);
    BOOST_CHECK(ret == true);
  }
}


BOOST_AUTO_TEST_CASE(TestStruct) {
  std::vector<A*> pointers;
  fix_sized_freelist<A> freelist(1024);

  for (int i = 0; i != 1024; ++i) {
    A* p = freelist.alloc();
    BOOST_CHECK(p != NULL);
    p->b = 1.123;
    pointers.push_back(p);
  }

  for (int i = 0; i != 1024; ++i) {
    A* p = freelist.alloc();
    BOOST_CHECK(p == NULL);
  }

  for(auto &p : pointers){
    bool ret = freelist.dealloc(p);
    BOOST_CHECK(ret == true);
  }
}

BOOST_AUTO_TEST_CASE(TestStructKey) {
  std::vector<uint64_t> keys;
  fix_sized_freelist<A> freelist(1024);

  for (int i = 0; i != 1024; ++i) {
    uint64_t key = freelist.alloc_key();
    BOOST_CHECK(key != 0);
    freelist.get_key(key)->b = 1.123;
    keys.push_back(key);
  }

  for (int i = 0; i != 1024; ++i) {
    uint64_t key = freelist.alloc_key();
    BOOST_CHECK(key == 0);
  }

  for(auto &p : keys){
    bool ret = freelist.dealloc_key(p);
    BOOST_CHECK(ret == true);
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
  uint64_t success_count = 0;
  uint64_t fail_count = 0;

  while(g_running_) {
    std::string* p = freelist->alloc();
    if (p && p->empty()) {
      if (!freelist->dealloc(p)) {
        std::cerr << "dealloc fail p:" << p << std::endl;
      } else {
        ++success_count;
        continue;
      }
    }
    ++fail_count;
  }

  std::cout << "success:" << success_count << ",fail:" << fail_count << std::endl;
}

void RunnerKey(fix_sized_freelist<std::string>* freelist) {
  uint64_t success_count = 0;
  uint64_t fail_count = 0;

  while(g_running_) {
    uint64_t key = freelist->alloc_key();
    std::string* p = freelist->get_key(key);
    if (p && p->empty()) {
      if (!freelist->dealloc_key(key)) {
        std::cerr << "dealloc fail key:" << key << std::endl;
      } else {
        ++success_count;
        continue;
      }
    }
    ++fail_count;
  }

  std::cout << "success:" << success_count << ",fail:" << fail_count << std::endl;
}

void RunnerNew() {
  uint64_t success_count = 0;
  uint64_t fail_count = 0;

  while(g_running_) {
    std::string* p = new std::string;
    if (p && p->empty()) {
      delete p;
      ++success_count;
      continue;
    }
    ++fail_count;
  }

  std::cout << "success:" << success_count << ",fail:" << fail_count << std::endl;
}

}


BOOST_AUTO_TEST_CASE(TestThreading) {
  std::vector<boost::thread*> threads;

  fix_sized_freelist<std::string> freelist(65536);
  g_running_ = true;

  for (size_t i = 0; i != 2; ++i) {
    boost::thread* t = new boost::thread(boost::bind(Runner, &freelist));
    threads.push_back(t);
  }

  sleep(20);
  g_running_ = false;

  for (size_t i = 0; i != threads.size(); ++i) {
    threads[i]->join();
    delete threads[i];
  }
}

BOOST_AUTO_TEST_CASE(TestThreadingKey) {
  std::vector<boost::thread*> threads;

  fix_sized_freelist<std::string> freelist(65536);
  g_running_ = true;

  for (size_t i = 0; i != 2; ++i) {
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

  for (size_t i = 0; i != 2; ++i) {
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



BOOST_AUTO_TEST_SUITE_END()
