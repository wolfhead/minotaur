/**
 * @file http_protocol.cpp
 * @author Wolfhead
 */
#include "http_protocol.h"
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include "../../io_buffer.h"
#include "../../io_message.h"
#include "../../../common/macro.h"

namespace minotaur {

namespace {

const static std::string field_x_heart_beat = "X-Heart-Beat";
const static std::string field_x_seq_id = "X-SEQ-ID";

static int OnMessageBegin(http_parser* parser) {
  HttpProtocolData* protocol_data = (HttpProtocolData*)parser;
  if (protocol_data->current) {
    MessageFactory::Destroy(protocol_data->current);
  }

  protocol_data->on_header_value = false;

  protocol_data->current = MessageFactory::Allocate<HttpMessage>();
  protocol_data->current->direction = ProtocolMessage::kDirectionUnknown;
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

  if (protocol_data->field == field_x_heart_beat) {
    protocol_data->current->type_id = MessageType::kHeartBeatMessage;
  }

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

  if (protocol_data->current->direction == ProtocolMessage::kDirectionUnknown) {
    protocol_data->current->direction = parser->type == HTTP_REQUEST 
      ? ProtocolMessage::kIncomingRequest
      : ProtocolMessage::kIncomingResponse;
  }

  protocol_data->current->http_major = parser->http_major;
  protocol_data->current->http_minor = parser->http_minor;
  protocol_data->current->status_code = parser->status_code;
  protocol_data->current->method = parser->method;
  protocol_data->current->keep_alive = http_should_keep_alive(parser);
  
  auto seq_id = protocol_data->current->header.find(field_x_seq_id);
  if (seq_id != protocol_data->current->header.end()) {
    protocol_data->current->sequence_id = strtoul(seq_id->second.c_str(), NULL, 10);
  }

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

const std::string& HttpProtocol::GetMethodString(int method) {
  static const std::string unknown("UNKNOWN");
  // we hack this implementation from http_parser.c
  static const std::string method_str[] = {
#define XX(num, name, string) #string,
    HTTP_METHOD_MAP(XX)
#undef XX
  };
  return ELEM_AT(method_str, method, unknown); 
}

const std::string& HttpProtocol::GetStatusString(int status) {
  static const std::string unknown("000 Unknown");
  static const std::string status_str_200[] = {
    "200 OK",
    "201 Created",
    "202 Accepted",
    "203 Non-Authoritative Information",
    "204 No Content",
    "205 Reset Content",
    "206 Partial Content",
    "207 Multi-Status",
  };
  static const std::string status_str_300[] = {
    "300 Multiple Choices",
    "301 Moved Permanently",
    "302 Move temporarily",
    "303 See Other",
    "304 Not Modified",
    "305 Use Proxy",
    "306 Switch Proxy",
    "307 Temporary Redirect",
  };
  static const std::string status_str_400[] = {
    "400 Bad Request",
    "401 Unauthorized",
    "402 Payment Required",
    "403 Forbidden",
    "404 Not Found",
    "405 Method Not Allowed",
    "406 Not Acceptable",
    "407 Proxy Authentication Required",
    "408 Request Timeout",
    "409 Conflict",
    "410 Gone",
    "411 Length Required",
    "412 Precondition Failed",
    "413 Request Entity Too Large",
    "414 Request-URI Too Long",
    "415 Unsupported Media Type",
    "416 Requested Range Not Satisfiable",
    "417 Expectation Failed",
  };
  static const std::string status_str_500[] = {
    "500 Internal Server Error",
    "501 Not Implemented",
    "502 Bad Gateway",
    "503 Service Unavailable",
    "504 Gateway Time-out",
    "506 HTTP Version not supported",
  };

  if (status < 200) {
    return unknown;
  } else if (status < 300) {
    return ELEM_AT(status_str_200, status - 200, unknown);
  } else if (status < 400) {
    return ELEM_AT(status_str_300, status - 300, unknown);
  } else if (status < 500) {
    return ELEM_AT(status_str_400, status - 400, unknown);
  } else if (status < 600) {
    return ELEM_AT(status_str_500, status - 500, unknown);
  } else {
    return unknown;
  }
}

HttpProtocol::HttpProtocol() 
    : Protocol(ProtocolType::kHttpProtocol, false) {
}

ProtocolMessage* HttpProtocol::Decode(
    IOBuffer* buffer, 
    int* result) {
  if (parser_.messages.size()) {
    ProtocolMessage* message = parser_.messages.front();
    parser_.messages.pop_front();
    *result = Protocol::kDecodeSuccess;
    return message;
  } 

  size_t parsed = http_parser_execute(
      (http_parser*)&parser_, 
      &HttpProtocolData::parser_setting, 
      buffer->GetRead(), 
      buffer->GetReadSize());

  if (parser_.parser.upgrade) {
    MI_LOG_ERROR(logger, "HttpProtocol::Decode http upgrade not supported");
    *result = Protocol::kDecodeFail;
    return NULL;
  } else if (parsed != buffer->GetReadSize()) {
    MI_LOG_ERROR(logger, "HttpProtocol::Decode fail:" 
        << ", fail: " << http_errno_description((http_errno)parser_.parser.http_errno)
        << ", buffer: " << std::string(buffer->GetCStyle()));
    *result = Protocol::kDecodeFail;
    return NULL;
  }

  buffer->Consume(parsed);

  if (parser_.messages.size()) {
    ProtocolMessage* message = parser_.messages.front();
    parser_.messages.pop_front();
    *result = Protocol::kDecodeSuccess;
    return message;
  } 

  *result = Protocol::kDecodeContinue;
  return NULL;
}

int HttpProtocol::Encode(
    IOBuffer* buffer,
    ProtocolMessage* message) {
#define WRITE_BUFFER_CONST(STR) \
  buffer->Write(STR, sizeof(STR) - 1);

  static const std::string field_content_length = "Content-Length";
  static const std::string field_connection = "Connection";

  HttpMessage* http_message = static_cast<HttpMessage*>(message);

  buffer->EnsureWrite(
      http_message->body.size() + 
      http_message->url.size() + 
      (http_message->header.size() << 6));

  if (http_message->direction == ProtocolMessage::kOutgoingResponse) {
    if (http_message->http_minor == 0) {
      WRITE_BUFFER_CONST("HTTP/1.0 ");
    } else {
      WRITE_BUFFER_CONST("HTTP/1.1 ");
    }

    buffer->WriteString(http_message->GetStatusString());
  } else {
    buffer->WriteString(http_message->GetMethodString());
    WRITE_BUFFER_CONST(" ");
    buffer->WriteString(http_message->url);
    WRITE_BUFFER_CONST(" ");
    if (http_message->http_minor == 0) {
      WRITE_BUFFER_CONST("HTTP/1.0");
    } else {
      WRITE_BUFFER_CONST("HTTP/1.1");
    }
  }
  WRITE_BUFFER_CONST("\r\n");

  // process header
  for (const auto& pair : http_message->header) {
    buffer->WriteString(pair.first);
    WRITE_BUFFER_CONST(":");
    buffer->WriteString(pair.second);
    WRITE_BUFFER_CONST("\r\n");
  }

  // add Content-Length if not exist
  if (http_message->header.find(field_content_length) == http_message->header.end()) {
    WRITE_BUFFER_CONST("Content-Length:");
    buffer->WriteString(boost::lexical_cast<std::string>(http_message->body.size()));
    WRITE_BUFFER_CONST("\r\n");
  } 

  // add Connection if not exist
  if (http_message->header.find(field_connection) == http_message->header.end()) {
    if (http_message->keep_alive && http_message->http_minor == 1) {
      WRITE_BUFFER_CONST("Connection:keep-alive\r\n");
    } else {
      WRITE_BUFFER_CONST("Connection:close\r\n");
    }
  }

  // add sequence id if not exist
  if (http_message->header.find(field_x_seq_id) == http_message->header.end()) {
    WRITE_BUFFER_CONST("X-SEQ-ID:");
    buffer->WriteString(boost::lexical_cast<std::string>(http_message->sequence_id));
    WRITE_BUFFER_CONST("\r\n");
  }

  WRITE_BUFFER_CONST("\r\n");
  buffer->WriteString(http_message->body);

  return Protocol::kEncodeSuccess;
#undef WRITE_BUFFER_CONST
}

ProtocolMessage* HttpProtocol::HeartBeatRequest() {
  static const std::string k_url = "/";
  static const std::string k_heart_beat_value = "true";

  HttpMessage* message = MessageFactory::Allocate<HttpMessage>();
  message->type_id = MessageType::kHeartBeatMessage;
  message->direction = ProtocolMessage::kOutgoingRequest;
  message->http_major = 1;
  message->http_minor = 1;
  message->keep_alive = true;
  message->method = kMethodGET;
  message->url = k_url;
  message->header.insert(std::make_pair(field_x_heart_beat, k_heart_beat_value));
  return message;
}

ProtocolMessage* HttpProtocol::HeartBeatResponse(ProtocolMessage* request) {
  HttpMessage* response = (HttpMessage*)request;

  response->direction = ProtocolMessage::kOutgoingResponse;
  response->status_code = 200;

  return response;
}

} //namespace minotaur
