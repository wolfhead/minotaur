#define BOOST_AUTO_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <net/protocol/redis/redis_parser.h>
#include <common/logger.h>
#include "unittest_logger.h"

using namespace ade;
using namespace ade::redis;
using namespace ade::unittest;

static ade::unittest::UnittestLogger logger_config;
LOGGER_STATIC_DECL_IMPL(logger, "root");

BOOST_AUTO_TEST_SUITE(TestRedisParser);

BOOST_AUTO_TEST_CASE(testRESPSize) {
  LOG_TRACE(logger, "RESP size:" << sizeof(RESP));
}

BOOST_AUTO_TEST_CASE(testRESPInteger) {
  RESP resp(123);

  BOOST_CHECK_EQUAL(resp.IsInteger(), true);
  BOOST_CHECK_EQUAL(resp.GetInteger(), 123);

  RESP resp_copy(resp);
  BOOST_CHECK_EQUAL(resp_copy.IsInteger(), true);
  BOOST_CHECK_EQUAL(resp_copy.GetInteger(), 123);
}

BOOST_AUTO_TEST_CASE(testRESPString) {
  RESP resp(RESP::kString, "test");

  BOOST_CHECK_EQUAL(resp.IsString(), true);
  BOOST_CHECK_EQUAL(resp.GetString(), "test");

  RESP resp_copy1(resp);
  RESP resp_copy2(resp_copy1);
  RESP resp_copy3(resp_copy2);
  RESP resp_copy(resp_copy3);
  BOOST_CHECK_EQUAL(resp_copy.IsString(), true);
  BOOST_CHECK_EQUAL(resp_copy.GetString(), "test");
}

BOOST_AUTO_TEST_CASE(testRESPArray) {
  RESP resp;
  resp.AppendInteger(123);
  resp.AppendInteger(234);
  resp.AppendString(RESP::kError, "test");
  resp.AppendString(RESP::kString, "test2");

  BOOST_CHECK_EQUAL(resp.IsArray(), true);
  BOOST_CHECK_EQUAL(resp.Size(), 4);
  BOOST_CHECK_EQUAL(resp.At(0).IsInteger(), true);
  BOOST_CHECK_EQUAL(resp.At(0).GetInteger(), 123);
  BOOST_CHECK_EQUAL(resp.At(1).IsInteger(), true);
  BOOST_CHECK_EQUAL(resp.At(1).GetInteger(), 234);
  BOOST_CHECK_EQUAL(resp.At(2).IsError(), true);
  BOOST_CHECK_EQUAL(resp.At(2).GetError(), "test");
  BOOST_CHECK_EQUAL(resp.At(3).IsString(), true);
  BOOST_CHECK_EQUAL(resp.At(3).GetString(), "test2");

  RESP resp_copy(resp);
  BOOST_CHECK_EQUAL(resp_copy.IsArray(), true);
  BOOST_CHECK_EQUAL(resp_copy.Size(), 4);
  BOOST_CHECK_EQUAL(resp_copy.At(0).IsInteger(), true);
  BOOST_CHECK_EQUAL(resp_copy.At(0).GetInteger(), 123);
  BOOST_CHECK_EQUAL(resp_copy.At(1).IsInteger(), true);
  BOOST_CHECK_EQUAL(resp_copy.At(1).GetInteger(), 234);
  BOOST_CHECK_EQUAL(resp_copy.At(2).IsError(), true);
  BOOST_CHECK_EQUAL(resp_copy.At(2).GetError(), "test");
  BOOST_CHECK_EQUAL(resp_copy.At(3).IsString(), true);
  BOOST_CHECK_EQUAL(resp_copy.At(3).GetString(), "test2");
}

BOOST_AUTO_TEST_CASE(testEncodeString) {
  std::string result;
  RedisParser::EncodeString("1234", &result);
  BOOST_CHECK_EQUAL(result, "+1234\r\n");
}

BOOST_AUTO_TEST_CASE(testEncodeError) {
  std::string result;
  RedisParser::EncodeError("1234", &result);
  BOOST_CHECK_EQUAL(result, "-1234\r\n");
}

BOOST_AUTO_TEST_CASE(testEncodeInteger) {
  std::string result;
  RedisParser::EncodeInteger(1234, &result);
  BOOST_CHECK_EQUAL(result, ":1234\r\n");
}

BOOST_AUTO_TEST_CASE(testBuldString) {
  std::string result;
  RedisParser::EncodeBulkString("1234", &result);
  BOOST_CHECK_EQUAL(result, "$4\r\n1234\r\n");
}

BOOST_AUTO_TEST_CASE(testArray) {
  std::string result;
  RedisParser::EncodeArray(2, &result);
  RedisParser::EncodeString("123", &result);
  RedisParser::EncodeBulkString("123", &result);
  BOOST_CHECK_EQUAL(result, "*2\r\n+123\r\n$3\r\n123\r\n");
}

BOOST_AUTO_TEST_CASE(testGetCommand) {
  int ret = 0;
  uint8_t type = RESP::kNull;
  Slice command;

  ret = RedisParser::GetCommand(Slice("+OK\r\n"), &type, &command);
  BOOST_CHECK_EQUAL(ret, 5);
  BOOST_CHECK_EQUAL(type, RESP::kString);
  BOOST_CHECK_EQUAL(command, "OK");

  ret = RedisParser::GetCommand(Slice("+\r\n"), &type, &command);
  BOOST_CHECK_EQUAL(ret, 3);
  BOOST_CHECK_EQUAL(type, RESP::kString);
  BOOST_CHECK_EQUAL(command, "");

  ret = RedisParser::GetCommand(Slice(":1234\r\n"), &type, &command);
  BOOST_CHECK_EQUAL(ret, 7);
  BOOST_CHECK_EQUAL(type, RESP::kInteger);
  BOOST_CHECK_EQUAL(command, "1234");

  ret = RedisParser::GetCommand(Slice("-error\r\n"), &type, &command);
  BOOST_CHECK_EQUAL(ret, 8);
  BOOST_CHECK_EQUAL(type, RESP::kError);
  BOOST_CHECK_EQUAL(command, "error");

  ret = RedisParser::GetCommand(Slice("*2\r\n"), &type, &command);
  BOOST_CHECK_EQUAL(ret, 4);
  BOOST_CHECK_EQUAL(type, RESP::kArray);
  BOOST_CHECK_EQUAL(command, "2");

  ret = RedisParser::GetCommand(Slice("$1024\r\n"), &type, &command);
  BOOST_CHECK_EQUAL(ret, 7);
  BOOST_CHECK_EQUAL(type, RESP::kBulkString);
  BOOST_CHECK_EQUAL(command, "1024");

  ret = RedisParser::GetCommand(Slice("-error"), &type, &command);
  BOOST_CHECK_EQUAL(ret, 0);
}

BOOST_AUTO_TEST_CASE(testGetBulkString) {
  int ret = 0;
  Slice bulk_string;

  ret = RedisParser::GetBulkString("123456\r\n", 6, &bulk_string);
  BOOST_CHECK_EQUAL(ret, 8);
  BOOST_CHECK_EQUAL(bulk_string, "123456");

  ret = RedisParser::GetBulkString("\r\n", 0, &bulk_string);
  BOOST_CHECK_EQUAL(ret, 2);
  BOOST_CHECK_EQUAL(bulk_string, "");

  ret = RedisParser::GetBulkString("\r\n", 12345676789, &bulk_string);
  BOOST_CHECK_EQUAL(ret, -1);

  ret = RedisParser::GetBulkString("\r\n", 1234, &bulk_string);
  BOOST_CHECK_EQUAL(ret, 0);

}

BOOST_AUTO_TEST_CASE(testSliceToInteger) {
  int ret = 0;
  int64_t result = 0;

  ret = RedisParser::SliceToInteger("123", &result);
  BOOST_CHECK_EQUAL(ret, 0);
  BOOST_CHECK_EQUAL(result, 123);

  ret = RedisParser::SliceToInteger("-1", &result);
  BOOST_CHECK_EQUAL(ret, 0);
  BOOST_CHECK_EQUAL(result, -1);

  ret = RedisParser::SliceToInteger("123a", &result);
  BOOST_CHECK_EQUAL(ret, -1);
}

BOOST_AUTO_TEST_CASE(testParseBulkString) {
  int ret = 0;
  RESP resp;

  ret = RedisParser::ParseBulkString("-1", Slice(), &resp);
  BOOST_CHECK_EQUAL(ret, 5);
  BOOST_CHECK_EQUAL(resp.IsNull(), true);

  resp.Reset();

  ret = RedisParser::ParseBulkString("10", "01234567890\r\n", &resp);
  BOOST_CHECK_EQUAL(ret, 17);
  BOOST_CHECK_EQUAL(resp.IsString(), true);
  BOOST_CHECK_EQUAL(resp.GetString(), "0123456789");

  resp.Reset();

  ret = RedisParser::ParseBulkString("3", "123\r\n", &resp);
  BOOST_CHECK_EQUAL(ret, 9);
  BOOST_CHECK_EQUAL(resp.IsString(), true);
  BOOST_CHECK_EQUAL(resp.GetString(), "123");

  resp.Reset();

  ret = RedisParser::ParseBulkString("4", "test\r\n", &resp);
  BOOST_CHECK_EQUAL(ret, 10);
  BOOST_CHECK_EQUAL(resp.IsString(), true);
  BOOST_CHECK_EQUAL(resp.GetString(), "test");
}

BOOST_AUTO_TEST_CASE(testParseArray) {
  int ret = 0;
  RESP resp;

  ret = RedisParser::ParseArray("0", Slice(), &resp);
  BOOST_CHECK_EQUAL(ret, 4);
  BOOST_CHECK_EQUAL(resp.IsArray(), true);
  BOOST_CHECK_EQUAL(resp.Size(), 0);

  resp.Reset();
  ret = RedisParser::ParseArray("2", "$4\r\ntest\r\n$3\r\n123\r\n", &resp);
  BOOST_CHECK_EQUAL(ret, 23);
  BOOST_CHECK_EQUAL(resp.IsArray(), true);
  BOOST_CHECK_EQUAL(resp.Size(), 2);
  BOOST_CHECK_EQUAL(resp.At(0).IsString(), true);
  BOOST_CHECK_EQUAL(resp.At(0).GetString(), "test");
}

BOOST_AUTO_TEST_CASE(testParse_Happy_1) {
  int ret = 0;
  RESP resp;
  ret = RedisParser::Parse("+OK\r\n", &resp);
  BOOST_CHECK_EQUAL(ret, 5);
  BOOST_CHECK_EQUAL(resp.IsString(), true);
  BOOST_CHECK_EQUAL(resp.GetString(), "OK");
}

BOOST_AUTO_TEST_CASE(testParse_Happy_2) {
  int ret = 0;
  RESP resp;
  ret = RedisParser::Parse("-Error\r\n", &resp);
  BOOST_CHECK_EQUAL(ret, 8);
  BOOST_CHECK_EQUAL(resp.IsError(), true);
  BOOST_CHECK_EQUAL(resp.GetError(), "Error");
}

BOOST_AUTO_TEST_CASE(testParse_Happy_3) {
  int ret = 0;
  RESP resp;
  ret = RedisParser::Parse(":12345\r\n", &resp);
  BOOST_CHECK_EQUAL(ret, 8);
  BOOST_CHECK_EQUAL(resp.IsInteger(), true);
  BOOST_CHECK_EQUAL(resp.GetInteger(), 12345);
}

BOOST_AUTO_TEST_CASE(testParse_Happy_4) {
  int ret = 0;
  RESP resp;
  ret = RedisParser::Parse("$-1\r\n", &resp);
  BOOST_CHECK_EQUAL(ret, 5);
  BOOST_CHECK_EQUAL(resp.IsNull(), true);
}

BOOST_AUTO_TEST_CASE(testParse_Happy_5) {
  int ret = 0;
  RESP resp;
  ret = RedisParser::Parse("$4\r\ntest\r\n", &resp);
  BOOST_CHECK_EQUAL(ret, 10);
  BOOST_CHECK_EQUAL(resp.IsString(), true);
  BOOST_CHECK_EQUAL(resp.GetString(), "test");
}

BOOST_AUTO_TEST_CASE(testParser_Continue_1) {
  int ret = 0;
  RESP resp;
  ret = RedisParser::Parse("", &resp);
  BOOST_CHECK_EQUAL(ret, 0);
}

BOOST_AUTO_TEST_CASE(testParser_Continue_2) {
  int ret = 0;
  RESP resp;
  ret = RedisParser::Parse("+OK\r", &resp);
  BOOST_CHECK_EQUAL(ret, 0);
}

BOOST_AUTO_TEST_CASE(testParser_Continue_3) {
  int ret = 0;
  RESP resp;
  ret = RedisParser::Parse("$10\r\r0123456789\r", &resp);
  BOOST_CHECK_EQUAL(ret, 0);
}

BOOST_AUTO_TEST_SUITE_END()
