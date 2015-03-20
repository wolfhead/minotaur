/**
 * @file http_protocol.cpp
 * @author Wolfhead
 */
#include "http_protocol.h"
#include <string.h>
#include <algorithm>
#include "../../io_buffer.h"
#include "../../io_message.h"

namespace minotaur {

namespace {

static int OnMessageBegin(http_parser* parser) {
  HttpProtocolData* protocol_data = (HttpProtocolData*)parser;
  if (protocol_data->current) {
    MessageFactory::Destroy(protocol_data->current);
  }
  protocol_data->current = MessageFactory::Allocate<HttpMessage>();
  return 0;
}

static int OnRequestUrl(http_parser* parser, const char* buf, size_t size) {
  HttpProtocolData* protocol_data = (HttpProtocolData*) parser;
  protocol_data->current->url.append(buf, size);
  return 0;
}

static int OnHeadersComplete(http_parser* parser) {
  HttpProtocolData* protocol_data = (HttpProtocolData*) parser;
  protocol_data->current->header.insert(std::make_pair(
        protocol_data->field,
        protocol_data->value
        ));

  protocol_data->field.clear();
  protocol_data->value.clear();
  return 0;
}

static int OnHeaderField(http_parser* parser, const char* buf, size_t size) {
  HttpProtocolData* protocol_data = (HttpProtocolData*) parser;
  if (protocol_data->on_header_value) {
    protocol_data->on_header_value = false;
    OnHeadersComplete(parser);
    protocol_data->field.assign(buf, size);
  } else {
    protocol_data->field.append(buf, size);
  }

  return 0;
}

static int OnHeaderValue(http_parser* parser, const char* buf, size_t size) {
  HttpProtocolData* protocol_data = (HttpProtocolData*) parser;
  if (protocol_data->on_header_value) {
    protocol_data->value.append(buf, size);
  } else {
    protocol_data->on_header_value = true;
    protocol_data->value.assign(buf, size);
  }
  return 0;
}

static int OnBody(http_parser* parser, const char* buf, size_t size) {
  HttpProtocolData* protocol_data = (HttpProtocolData*) parser;
  protocol_data->current->body.append(buf, size);
  return 0;
}

static int OnMessageComplete(http_parser* parser) {
  HttpProtocolData* protocol_data = (HttpProtocolData*) parser;

  protocol_data->current->http_major = parser->http_major;
  protocol_data->current->http_minor = parser->http_minor;
  protocol_data->current->status_code = parser->status_code;
  protocol_data->current->method = parser->method;
  protocol_data->current->keep_alive = http_should_keep_alive(parser);
  protocol_data->current->http_type = parser->type;
  
  protocol_data->messages.push_back(protocol_data->current);
  protocol_data->current = NULL;
  return 0;
}

} //namespace 

HttpProtocolData::~HttpProtocolData() {
  if (current) {
    MessageFactory::Destroy(current);
  }
}

struct http_parser_settings HttpProtocolData::parser_setting = {
  .on_message_begin = &OnMessageBegin,
  .on_url = &OnRequestUrl,
  .on_status = NULL,
  .on_header_field = &OnHeaderField,
  .on_header_value = &OnHeaderValue,
  .on_headers_complete = &OnHeadersComplete,
  .on_body = &OnBody,
  .on_message_complete = &OnMessageComplete,
};

LOGGER_CLASS_IMPL_NAME(logger, HttpProtocol, "net.HttpProtocol");

HttpProtocol::HttpProtocol() 
    : Protocol(ProtocolType::kHttpProtocol, false) {
}

ProtocolMessage* HttpProtocol::Decode(
    IODescriptor* descriptor, 
    IOBuffer* buffer, 
    int* result) {
  if (parser_.messages.size()) {
    ProtocolMessage* message = parser_.messages.front();
    parser_.messages.pop_front();
    *result = Protocol::kResultDecoded;
    return message;
  } 

  size_t parsed = http_parser_execute(
      (http_parser*)&parser_, 
      &HttpProtocolData::parser_setting, 
      buffer->GetRead(), 
      buffer->GetReadSize());

  if (parser_.parser.upgrade) {
    MI_LOG_ERROR(logger, "HttpProtocol::Decode http upgrade not supported");
    *result = Protocol::kResultFail;
    return NULL;
  } else if (parsed != buffer->GetReadSize()) {
    MI_LOG_ERROR(logger, "HttpProtocol::Decode fail:" 
        << ", fail: "<< http_errno_description((http_errno)parser_.parser.http_errno));
    *result = Protocol::kResultFail;
    return NULL;
  }

  buffer->Consume(parsed);

  if (parser_.messages.size()) {
    ProtocolMessage* message = parser_.messages.front();
    parser_.messages.pop_front();
    *result = Protocol::kResultDecoded;
    return message;
  } 

  *result = Protocol::kResultContinue;
  return NULL;
}

bool HttpProtocol::Encode(
    IODescriptor* descriptor,
    IOBuffer* buffer,
    ProtocolMessage* message) {
  static const std::string response = 
    "HTTP/1.1 200 OK\r\n"
    "Content-Length: 4\r\n"
    //"Connection: Keep-Alive\r\n"
    "Connection: Keep-Alive\r\n"
    "\r\n"
    "pong";

  HttpMessage* http_message = static_cast<HttpMessage*>(message);

  buffer->EnsureWrite(response.size());
  buffer->Write(response.data(), response.size());
  return true;
}

} //namespace minotaur
