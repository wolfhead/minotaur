#define BOOST_AUTO_TEST_MAIN

#include <boost/thread.hpp>
#include <boost/test/unit_test.hpp>
#include <net/io_descriptor_factory.h>
#include <net/protocol/protocol_factory.h>
#include "unittest_logger.h"

using namespace minotaur;
using namespace minotaur::unittest;

static minotaur::unittest::UnittestLogger logger_config;
LOGGER_STATIC_DECL_IMPL(logger, "root");

BOOST_AUTO_TEST_SUITE(TestIODescriptorFactory);

BOOST_AUTO_TEST_CASE(testParseAddress) {
  std::string ip;
  int port;
  int protocol;
  int ret = 0;

  ret = IODescriptorFactory::ParseAddress("http://0.0.0.0:80", &ip, &port, &protocol);
  BOOST_CHECK_EQUAL(ret, 0);
  BOOST_CHECK_EQUAL(ip, "0.0.0.0");
  BOOST_CHECK_EQUAL(port, 80);
  BOOST_CHECK_EQUAL(protocol, ProtocolType::kHttpProtocol);

  ret = IODescriptorFactory::ParseAddress("rapid://localhost:9999", &ip, &port, &protocol);
  BOOST_CHECK_EQUAL(ret, 0);
  BOOST_CHECK_EQUAL(ip, "127.0.0.1");
  BOOST_CHECK_EQUAL(port, 9999);
  BOOST_CHECK_EQUAL(protocol, ProtocolType::kRapidProtocol);

}

BOOST_AUTO_TEST_SUITE_END()

