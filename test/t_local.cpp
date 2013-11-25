#define BOOST_AUTO_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <local.hpp>

BOOST_AUTO_TEST_SUITE(TestLocal);

namespace {

static int g_alive_count = 0;
void clear_alive_count() {
  g_alive_count = 0;
}

int get_alive_count() {
  return g_alive_count;
}


class A {
 public:
  A(int i) : value_(i) {
    std::cout << "A construct:" << value_ << std::endl;
    ++g_alive_count;
  }

  virtual ~A() {
    std::cout << "A destruct:" << value_ << std::endl;
    --g_alive_count;
  }

  int value() const {return value_;}
  void value(int i) {value_ = i;}

 protected:
  int value_;
};

class B : public A {
 public:
  B(int i) : A(i) {
    std::cout << "B construct:" << value_ << std::endl;
  }

  ~B() {
    std::cout << "B destruct:" << value_ << std::endl;
  }
};

} // namespace 

BOOST_AUTO_TEST_CASE(TestCompile) {
  clear_alive_count();

  {
    mt::local<int> p_int(new int(0));
    mt::local<A> p_a(new A(1));
    mt::local<A> p_b(new B(1));
  }

  BOOST_CHECK_EQUAL(0, get_alive_count());
}

BOOST_AUTO_TEST_CASE(TestMemberFunction) {
  clear_alive_count();

  {
    mt::local<A> p_a(new A(1));
    BOOST_CHECK_EQUAL(1, p_a->value());
    p_a->value(2);
    BOOST_CHECK_EQUAL(2, p_a->value());
  }

  BOOST_CHECK_EQUAL(0, get_alive_count());
}

BOOST_AUTO_TEST_CASE(TestReset) {
  clear_alive_count();
  {
    mt::local<A> p_a(new A(1));
    p_a.reset(new B(2));
    BOOST_CHECK_EQUAL(2, p_a->value());
  }
  BOOST_CHECK_EQUAL(0, get_alive_count());
}

BOOST_AUTO_TEST_CASE(TestClear) {
  clear_alive_count();
  {
    mt::local<A> p_a(new A(1));
    p_a.clear();

    BOOST_CHECK_EQUAL(true, NULL == p_a.get());
  }
  BOOST_CHECK_EQUAL(0, get_alive_count());
}

BOOST_AUTO_TEST_CASE(TestTakeover) {
  clear_alive_count();
  {
    mt::local<A> p_a(new A(1));
    A* p = p_a.takeover();

    BOOST_CHECK_EQUAL(true, NULL == p_a.get());
    BOOST_CHECK_EQUAL(1, p->value());
  }
  BOOST_CHECK_EQUAL(1, get_alive_count());
}

BOOST_AUTO_TEST_SUITE_END()