#define BOOST_AUTO_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <matrix/matrix.h>
#include <matrix/matrix_scope.h>
#include <common/logger.h>
#include "unittest_logger.h"

using namespace ade;
using namespace ade::matrix;
using namespace ade::unittest;

static ade::unittest::UnittestLogger logger_config;
LOGGER_STATIC_DECL_IMPL(logger, "root");

BOOST_AUTO_TEST_SUITE(TestMatrix);

BOOST_AUTO_TEST_CASE(testInit) {
  GlobalMatrix::Init(32, 32, 1024 * 128);
}

BOOST_AUTO_TEST_CASE(testSet) {
  GlobalMatrix::Instance().Set("test1", 1);
}

BOOST_AUTO_TEST_CASE(testAdd) {
  GlobalMatrix::Instance().Add("test2", 1);
}

BOOST_AUTO_TEST_CASE(testSub) {
  GlobalMatrix::Instance().Sub("test3", 1);
}

BOOST_AUTO_TEST_CASE(testReset) {
  GlobalMatrix::Instance().Reset("test4");
}

BOOST_AUTO_TEST_CASE(testScope) {
  int i = 60;
  while (i--) {
    MatrixScope scope("test_scope");
    sleep(1);
    scope.SetOkay(true);
  }
}

BOOST_AUTO_TEST_CASE(testStop) {
  sleep(5);
}

BOOST_AUTO_TEST_SUITE_END();
