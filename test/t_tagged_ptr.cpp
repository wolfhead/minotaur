#define BOOST_AUTO_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <lockfree/tagged_ptr.hpp>

using namespace ade::lockfree;

BOOST_AUTO_TEST_SUITE(TestTaggedPtr);

BOOST_AUTO_TEST_CASE(Test) {
  int a = 123;

  tagged_ptr<int> p1(&a, 0);
  tagged_ptr<int> p2(&a, 1);

  BOOST_CHECK_EQUAL(false, p1 == p2);
  BOOST_CHECK_EQUAL(123, *p1);
  BOOST_CHECK_EQUAL(123, *p2);

  BOOST_CHECK_EQUAL(true, p1.next_tag() == p2);
}

BOOST_AUTO_TEST_SUITE_END()
