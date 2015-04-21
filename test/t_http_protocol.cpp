#define BOOST_AUTO_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <net/protocol/http/http_protocol.h>
#include <net/io_buffer.h>
#include <net/io_message.h>
#include <common/logger.h>
#include "unittest_logger.h"

using namespace minotaur;
using namespace minotaur::unittest;

static minotaur::unittest::UnittestLogger logger_config;
LOGGER_STATIC_DECL_IMPL(logger, "root");

BOOST_AUTO_TEST_SUITE(TestHttpProtocol);

BOOST_AUTO_TEST_CASE(testHttpMethod) {
  BOOST_CHECK_EQUAL(HttpProtocol::GetMethodString(HttpProtocol::kMethodGET), "GET");
  BOOST_CHECK_EQUAL(HttpProtocol::GetMethodString(HttpProtocol::kMethodPOST), "POST");
  BOOST_CHECK_EQUAL(HttpProtocol::GetMethodString(HttpProtocol::kMethodPUT), "PUT");
  BOOST_CHECK_EQUAL(HttpProtocol::GetMethodString(-1), "UNKNOWN");
  BOOST_CHECK_EQUAL(HttpProtocol::GetMethodString(999), "UNKNOWN");
}

BOOST_AUTO_TEST_CASE(testHttpStatus) {
  BOOST_CHECK_EQUAL(HttpProtocol::GetStatusString(-1), "000 Unknown");
  BOOST_CHECK_EQUAL(HttpProtocol::GetStatusString(100), "000 Unknown");
  BOOST_CHECK_EQUAL(HttpProtocol::GetStatusString(200), "200 OK");
  BOOST_CHECK_EQUAL(HttpProtocol::GetStatusString(304), "304 Not Modified");
  BOOST_CHECK_EQUAL(HttpProtocol::GetStatusString(404), "404 Not Found");
  BOOST_CHECK_EQUAL(HttpProtocol::GetStatusString(502), "502 Bad Gateway");
  BOOST_CHECK_EQUAL(HttpProtocol::GetStatusString(507), "000 Unknown");
}

BOOST_AUTO_TEST_CASE(testDecodeRequest) {
  std::string data;
  IOBuffer buffer;
  int result = 0;
  HttpMessage* message = NULL;
  HttpProtocol http_protocol;

  data.append("GET / HTTP/1.1\r\n")
      .append("Host: localhost\r\n")
      .append("User-Agent: test\r\n")
      .append("Content-Length: 4\r\n")
      .append("Connection: keep-alive\r\n")
      .append("X-SEQ-ID: 1234\r\n")
      .append("\r\n")
      .append("body");

  buffer.WriteString(data); 
  message = (HttpMessage*)http_protocol.Decode(&buffer, &result);

  BOOST_CHECK_EQUAL(result, Protocol::kDecodeSuccess);
  BOOST_CHECK(message != NULL);
  BOOST_CHECK_EQUAL(message->http_major, 1);
  BOOST_CHECK_EQUAL(message->http_minor, 1);
  BOOST_CHECK_EQUAL(message->direction, ProtocolMessage::kIncomingRequest);
  BOOST_CHECK_EQUAL(message->url, "/");
  BOOST_CHECK_EQUAL(message->method, HttpProtocol::kMethodGET);
  BOOST_CHECK_EQUAL(message->keep_alive, true);
  BOOST_CHECK_EQUAL(message->header["Connection"], "keep-alive");
  BOOST_CHECK_EQUAL(message->header["Host"], "localhost");
  BOOST_CHECK_EQUAL(message->header["Content-Length"], "4");
  BOOST_CHECK_EQUAL(message->header["User-Agent"], "test");
  BOOST_CHECK_EQUAL(message->sequence_id, 1234);
  BOOST_CHECK_EQUAL(message->body, "body");
  BOOST_CHECK_EQUAL(message->header.size(), 5);
}

BOOST_AUTO_TEST_CASE(testDecodeRequestiContinue) {
  std::string data;
  IOBuffer buffer;
  int result = 0;
  HttpMessage* message = NULL;
  HttpProtocol http_protocol;

  data.append("GET / HTTP/1.1\r\n")
      .append("Host: localhost\r\n")
      .append("User-Agent: test\r\n");

  buffer.WriteString(data); 
  message = (HttpMessage*)http_protocol.Decode(&buffer, &result);

  BOOST_CHECK(message == NULL);
  BOOST_CHECK_EQUAL(result, Protocol::kDecodeContinue);

  data.clear();
  data.append("Content-Length: 4\r\n")
      .append("Connection: keep-alive\r\n")
      .append("X-SEQ-ID: 1234\r\n")
      .append("\r\n")
      .append("body");

  buffer.WriteString(data); 
  message = (HttpMessage*)http_protocol.Decode(&buffer, &result);

  BOOST_CHECK_EQUAL(result, Protocol::kDecodeSuccess);
  BOOST_CHECK(message != NULL);
  BOOST_CHECK_EQUAL(message->http_major, 1);
  BOOST_CHECK_EQUAL(message->http_minor, 1);
  BOOST_CHECK_EQUAL(message->direction, ProtocolMessage::kIncomingRequest);
  BOOST_CHECK_EQUAL(message->url, "/");
  BOOST_CHECK_EQUAL(message->method, HttpProtocol::kMethodGET);
  BOOST_CHECK_EQUAL(message->keep_alive, true);
  BOOST_CHECK_EQUAL(message->header["Connection"], "keep-alive");
  BOOST_CHECK_EQUAL(message->header["Host"], "localhost");
  BOOST_CHECK_EQUAL(message->header["Content-Length"], "4");
  BOOST_CHECK_EQUAL(message->header["User-Agent"], "test");
  BOOST_CHECK_EQUAL(message->sequence_id, 1234);
  BOOST_CHECK_EQUAL(message->body, "body");
}

BOOST_AUTO_TEST_CASE(testDecodeRequestHeartbeat) {
  std::string data;
  IOBuffer buffer;
  int result = 0;
  HttpMessage* message = NULL;
  HttpProtocol http_protocol;

  data.append("GET / HTTP/1.1\r\n")
      .append("Host: localhost\r\n")
      .append("Connection: keep-alive\r\n")
      .append("X-SEQ-ID: 1234\r\n")
      .append("X-Heart-Beat: true\r\n")
      .append("\r\n");

  buffer.WriteString(data); 
  message = (HttpMessage*)http_protocol.Decode(&buffer, &result);

  BOOST_CHECK_EQUAL(result, Protocol::kDecodeSuccess);
  BOOST_CHECK(message != NULL);
  BOOST_CHECK_EQUAL(message->type_id, MessageType::kHeartBeatMessage);
  BOOST_CHECK_EQUAL(message->http_major, 1);
  BOOST_CHECK_EQUAL(message->http_minor, 1);
  BOOST_CHECK_EQUAL(message->direction, ProtocolMessage::kIncomingRequest);
  BOOST_CHECK_EQUAL(message->url, "/");
  BOOST_CHECK_EQUAL(message->method, HttpProtocol::kMethodGET);
  BOOST_CHECK_EQUAL(message->keep_alive, true);
  BOOST_CHECK_EQUAL(message->sequence_id, 1234);
}

BOOST_AUTO_TEST_CASE(testDecodeResponse) {
  std::string data;
  IOBuffer buffer;
  int result = 0;
  HttpMessage* message = NULL;
  HttpProtocol http_protocol;

  data.append("HTTP/1.1 200 OK\r\n")
      .append("Content-Length: 4\r\n")
      .append("Connection: keep-alive\r\n")
      .append("X-SEQ-ID: 1234\r\n")
      .append("\r\n")
      .append("body");

  buffer.WriteString(data); 
  message = (HttpMessage*)http_protocol.Decode(&buffer, &result);

  BOOST_CHECK_EQUAL(result, Protocol::kDecodeSuccess);
  BOOST_CHECK(message != NULL);
  BOOST_CHECK_EQUAL(message->http_major, 1);
  BOOST_CHECK_EQUAL(message->http_minor, 1);
  BOOST_CHECK_EQUAL(message->direction, ProtocolMessage::kIncomingResponse);
  BOOST_CHECK_EQUAL(message->keep_alive, true);
  BOOST_CHECK_EQUAL(message->header["Connection"], "keep-alive");
  BOOST_CHECK_EQUAL(message->header["Content-Length"], "4");
  BOOST_CHECK_EQUAL(message->sequence_id, 1234);
  BOOST_CHECK_EQUAL(message->body, "body");
}

BOOST_AUTO_TEST_CASE(testEncodeRequest) {
  HttpMessage message;
  HttpProtocol protocol;
  IOBuffer buffer;

  message.direction = ProtocolMessage::kOutgoingRequest;
  message.sequence_id = 1234;
  message.http_major = 1;
  message.http_minor = 1;
  message.keep_alive = true;
  message.method = HttpProtocol::kMethodGET;
  message.url = "/test";
  message.header["Host"] = "localhost";

  int result = protocol.Encode(&buffer, &message);
  BOOST_CHECK_EQUAL(result, Protocol::kEncodeSuccess);

  HttpMessage* result_message = NULL;
  result_message = (HttpMessage*)protocol.Decode(&buffer, &result);
  BOOST_CHECK_EQUAL(result, Protocol::kDecodeSuccess);
  BOOST_CHECK(result_message != NULL);
  BOOST_CHECK_EQUAL(result_message->sequence_id, 1234);
  BOOST_CHECK_EQUAL(result_message->url, "/test");
  BOOST_CHECK_EQUAL(result_message->keep_alive, true);
  BOOST_CHECK_EQUAL(result_message->http_major, 1);
  BOOST_CHECK_EQUAL(result_message->http_minor, 1);
}


BOOST_AUTO_TEST_SUITE_END();
