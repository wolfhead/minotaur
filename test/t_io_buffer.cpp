#define BOOST_AUTO_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <net/io_buffer.h>
#include <common/logger.h>
#include "unittest_logger.h"

using namespace minotaur;
using namespace minotaur::unittest;

static minotaur::unittest::UnittestLogger logger_config;
LOGGER_STATIC_DECL_IMPL(logger, "root");

BOOST_AUTO_TEST_SUITE(TestIOBuffer);

BOOST_AUTO_TEST_CASE(testSmallChunk) {
  IOBuffer buffer;
  std::string test_data("1234567890");

  int i = 1000;
  while (i--) {
    buffer.Write(test_data.data(), test_data.size());
    BOOST_CHECK_EQUAL(buffer.GetReadSize(), test_data.size());
    std::string tmp(buffer.GetRead(), buffer.GetReadSize());
    BOOST_CHECK_EQUAL(tmp, test_data);
    buffer.Consume(tmp.size());
  }
}


BOOST_AUTO_TEST_CASE(testSmallChunkBatch) {
  IOBuffer buffer;
  std::string test_data("1234567890");

  int i = 1000;
  while (i--) {
    buffer.Write(test_data.data(), test_data.size());
  }
  i = 1000;
  while (i--) {
    std::string tmp(buffer.GetRead(), 10);
    BOOST_CHECK_EQUAL(tmp, test_data);
    buffer.Consume(tmp.size());
  }
}

BOOST_AUTO_TEST_SUITE_END()
