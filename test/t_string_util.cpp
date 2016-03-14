#define BOOST_AUTO_TEST_MAIN

#include <arpa/inet.h>
#include <boost/test/unit_test.hpp>
#include "../source/minotaur/common/string_util.h"
#include "../source/minotaur/common/logger.h"
#include "unittest_logger.h"

using namespace ade;
using namespace ade::unittest;

static ade::unittest::UnittestLogger logger_config;
LOGGER_STATIC_DECL_IMPL(logger, "root");

BOOST_AUTO_TEST_SUITE(TestStringUtil);

BOOST_AUTO_TEST_CASE(testToHex) {
  int test = 0x12345678;
  test = htonl(test);
  std::string result = strutil::ToHex(&test, sizeof(test));

  BOOST_CHECK_EQUAL(result, "12345678");
}

BOOST_AUTO_TEST_CASE(testUrlEncode) {
  BOOST_CHECK_EQUAL(
      strutil::UrlEncode("http://www.baodi.com/?test=123&key=cvb"), 
      "http%3a%2f%2fwww.baodi.com%2f%3ftest%3d123%26key%3dcvb");
}

BOOST_AUTO_TEST_CASE(testUrlDecode) {
  BOOST_CHECK_EQUAL(
      strutil::UrlDecode("http%3a%2f%2fwww.baodi.com%2f%3ftest%3d123%26key%3dcvb"),
      "http://www.baodi.com/?test=123&key=cvb");
}

BOOST_AUTO_TEST_CASE(testMd5) {
  BOOST_CHECK_EQUAL(
      strutil::ToHex(strutil::Md5("test_string"), true),
      "3474851A3410906697EC77337DF7AAE4");
}

BOOST_AUTO_TEST_CASE(testBase64Encode) {
  BOOST_CHECK_EQUAL(
      strutil::Base64Encode("any carnal pleasure."),
      "YW55IGNhcm5hbCBwbGVhc3VyZS4="); 
  BOOST_CHECK_EQUAL(
      strutil::Base64Encode("any carnal pleasure"),
      "YW55IGNhcm5hbCBwbGVhc3VyZQ=="); 
  BOOST_CHECK_EQUAL(
      strutil::Base64Encode("any carnal pleasur"),
      "YW55IGNhcm5hbCBwbGVhc3Vy"); 
  BOOST_CHECK_EQUAL(
      strutil::Base64Encode("any carnal pleasu"),
      "YW55IGNhcm5hbCBwbGVhc3U="); 
  BOOST_CHECK_EQUAL(
      strutil::Base64Encode("any carnal pleas"),
      "YW55IGNhcm5hbCBwbGVhcw=="); 
}

BOOST_AUTO_TEST_CASE(testBase64Decode) {
  BOOST_CHECK_EQUAL(
      "any carnal pleasure.",
      strutil::Base64Decode("YW55IGNhcm5hbCBwbGVhc3VyZS4=")); 
  BOOST_CHECK_EQUAL(
      "any carnal pleasure",
      strutil::Base64Decode("YW55IGNhcm5hbCBwbGVhc3VyZQ==")); 
  BOOST_CHECK_EQUAL(
      "any carnal pleasur",
      strutil::Base64Decode("YW55IGNhcm5hbCBwbGVhc3Vy")); 
  BOOST_CHECK_EQUAL(
      "any carnal pleasu",
      strutil::Base64Decode("YW55IGNhcm5hbCBwbGVhc3U=")); 
  BOOST_CHECK_EQUAL(
      "any carnal pleas",
      strutil::Base64Decode("YW55IGNhcm5hbCBwbGVhcw==")); 

  BOOST_CHECK_EQUAL(
      "any carnal pleasure.",
      strutil::Base64Decode("YW55IGNhcm5hbCBwbGVhc3VyZS4")); 
  BOOST_CHECK_EQUAL(
      "any carnal pleas",
      strutil::Base64Decode("YW55IGNhcm5hbCBwbGVhcw"));
}

BOOST_AUTO_TEST_CASE(testRandomString) {
  LOG_TRACE(logger, "random:" << strutil::RandomString(32));
}

BOOST_AUTO_TEST_CASE(testCRC32) {
  std::string ua = "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/46.0.2490.86 Safari/537.36";
  std::string ip = "1709574626";
  BOOST_CHECK_EQUAL(693920631, strutil::CRC32("11112222"));
  BOOST_CHECK_EQUAL(2356372769, strutil::CRC32("foo"));
  LOG_TRACE(logger, "str:" << strutil::ToHex(strutil::Md5(ip + ua)));
}

BOOST_AUTO_TEST_SUITE_END()
