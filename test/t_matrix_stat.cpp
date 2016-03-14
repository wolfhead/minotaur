#define BOOST_AUTO_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <matrix/matrix_stat.h>
#include <common/logger.h>
#include "unittest_logger.h"

using namespace ade;
using namespace ade::matrix;
using namespace ade::unittest;

static ade::unittest::UnittestLogger logger_config;
LOGGER_STATIC_DECL_IMPL(logger, "root");

BOOST_AUTO_TEST_SUITE(TestMatrixStat);

BOOST_AUTO_TEST_CASE(testAvg) {
  MatrixStat stat(time(NULL), false);
  BOOST_CHECK_EQUAL(0, stat.GetAvg());

  stat.TimeDistribute(1000);
  stat.TimeDistribute(2000);
  stat.TimeDistribute(3000);
  stat.TimeDistribute(4000);
  stat.TimeDistribute(5000);

  BOOST_CHECK_EQUAL(5, stat.GetCount());
  BOOST_CHECK_EQUAL(15000, stat.GetValue());
  BOOST_CHECK_EQUAL(3000, stat.GetAvg());
}

BOOST_AUTO_TEST_CASE(testQps) {
  MatrixStat stat(time(NULL) - 5, false);
  BOOST_CHECK_EQUAL(0, stat.GetQps());

  stat.TimeDistribute(1000);
  stat.TimeDistribute(2000);
  stat.TimeDistribute(3000);
  stat.TimeDistribute(4000);
  stat.TimeDistribute(5000);

  BOOST_CHECK_EQUAL(1, stat.GetQps()); 
}

BOOST_AUTO_TEST_CASE(testTimeDistribute) {
  MatrixStat stat(time(NULL), false);

  for (int i = 0; i != 50; ++i) {
    stat.TimeDistribute(100);
  }
  for (int i = 0; i != 30; ++i) {
    stat.TimeDistribute(200);
  }
  for (int i = 0; i != 10; ++i) {
    stat.TimeDistribute(300);
  }
  for (int i = 0; i != 5; ++i) {
    stat.TimeDistribute(400);
  }
  for (int i = 0; i != 5; ++i) {
    stat.TimeDistribute(500);
  }

  BOOST_CHECK_EQUAL(100, stat.GetTimeDistribute(0.5));
  BOOST_CHECK_EQUAL(200, stat.GetTimeDistribute(0.8));
  BOOST_CHECK_EQUAL(300, stat.GetTimeDistribute(0.9));
  BOOST_CHECK_EQUAL(400, stat.GetTimeDistribute(0.95));
  BOOST_CHECK_EQUAL(500, stat.GetTimeDistribute(0.99));

  LOG_TRACE(logger, "testTimeDistribute:" << stat);
}

BOOST_AUTO_TEST_SUITE_END()
