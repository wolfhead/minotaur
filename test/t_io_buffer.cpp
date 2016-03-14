#define BOOST_AUTO_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <net/io_buffer.h>
#include <common/logger.h>
#include "unittest_logger.h"

using namespace ade;
using namespace ade::unittest;

static ade::unittest::UnittestLogger logger_config;
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

BOOST_AUTO_TEST_CASE(testSmallChunk2) {
  IOBuffer buffer;
  std::string test_data("1234567890");

  int i = 2000000;
  while (i--) {
    char* p = buffer.EnsureWrite(4096);
    memcpy(p, test_data.data(), test_data.size());
    buffer.Produce(test_data.size());

    if (i % 2) {
      std::string tmp(buffer.GetRead(), test_data.size());
      BOOST_CHECK_EQUAL(tmp, test_data);
      buffer.Consume(tmp.size());
    }
  }

  i = 1000000;
  while (i--) {
    std::string tmp(buffer.GetRead(), test_data.size());
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


BOOST_AUTO_TEST_CASE(testLargeChunk) {
  IOBuffer buffer;
  std::string test_data("1234567890");

  int i = 7;
  while (i--) {
    test_data.append(test_data);
  }

  std::cout << test_data.size() << std::endl;
  i = 2000000;
  int count = 0;
  while (i--) {
    char* p = buffer.EnsureWrite(4096);
    uint32_t last_size = buffer.GetReadSize();
    memcpy(p, test_data.data(), test_data.size());
    buffer.Produce(test_data.size());
    BOOST_CHECK_EQUAL(buffer.GetWrite() - p, test_data.size());
    BOOST_CHECK_EQUAL(buffer.GetWrite() - p, 1280);
    BOOST_CHECK_EQUAL(buffer.GetReadSize() - last_size, 1280);


    if (i % 2) {
      char* p = buffer.GetRead();
      uint32_t last_size = buffer.GetReadSize();

      ++count;
      std::string tmp(buffer.GetRead(), test_data.size());
      buffer.Consume(tmp.size());

      BOOST_CHECK_EQUAL(tmp, test_data);
      BOOST_CHECK_EQUAL(last_size - buffer.GetReadSize(), 1280);
    }
  }

  i = 1000000;
  while (i--) {
    std::string tmp(buffer.GetRead(), test_data.size());
    BOOST_CHECK_EQUAL(tmp, test_data);
    buffer.Consume(tmp.size());
  }
}

BOOST_AUTO_TEST_SUITE_END()
