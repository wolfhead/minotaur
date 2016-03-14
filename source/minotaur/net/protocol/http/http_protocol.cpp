/**
 * @file http_protocol.cpp
 * @author Wolfhead
 */
#include "http_protocol.h"
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include "../../io_buffer.h"
#include "../../io_message.h"
#include "../../../common/macro.h"
#include "../../../common/string_util.h"

namespace ade {

namespace {

const static std::string field_x_heart_beat = "X-Heart-Beat";
const static std::string field_x_seq_id = "X-SEQ-ID";
const static std::string field_accept_encoding = "Accept-Encoding";
const static std::string k_value_gzip = "gzip";

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
  } else if (protocol_data->field == field_accept_encoding) {
    if (protocol_data->value.find(k_value_gzip) != std::string::npos) {
      protocol_data->current->gzip = true;
    }
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
  } else {
    protocol_data->current->sequence_id = 0;
  }

  protocol_data->messages.push_back(protocol_data->current);
  protocol_data->current = NULL;
  return 0;
}

} //namespace 

HttpProtocolData::HttpProtocolData() 
  : current(NULL) 
  , on_header_value(false) {
  http_parser_init((http_parser*)this, HTTP_BOTH);
}

HttpProtocolData::~HttpProtocolData() {
  if (current) {
    MessageFactory::Destroy(current);
  }
}

void HttpProtocolData::Reset() {
  http_parser_init((http_parser*)this, HTTP_BOTH);
  messages.clear();
  if (current) {
    MessageFactory::Destroy(current);
    current = NULL;
  }
  field.clear();
  value.clear();
  on_header_value = false;
}

void SliceHttpUrlEntity::Dump(std::ostream& os) const {
  os << "{\"schema\": \"" << schema << "\""
     << ", \"host\": \"" << host << "\""
     << ", \"port\": \"" << port << "\""
     << ", \"path\": \"" << path << "\""
     << ", \"query\": \"" << query << "\""
     << ", \"fragment\": \"" << fragment << "\""
     << ", \"userinfo\": \"" << userinfo << "\""
     << ", \"param\": {";
  bool first = true;
  for (const auto& pair : params) {
    if (first) {
      first = false;
    } else {
      os << ", ";
    }
    os << "\"" << pair.first << "\": \"" << pair.second << "\"";
  }
  os << "}";
}

HttpUrlEntity::HttpUrlEntity() {
}

HttpUrlEntity::HttpUrlEntity(const SliceHttpUrlEntity& entity) {
  BuildFromSlice(entity);
}

void HttpUrlEntity::Dump(std::ostream& os) const {
  os << "{\"schema\": \"" << schema << "\""
     << ", \"host\": \"" << host << "\""
     << ", \"port\": \"" << port << "\""
     << ", \"path\": \"" << path << "\""
     << ", \"query\": \"" << query << "\""
     << ", \"fragment\": \"" << fragment << "\""
     << ", \"userinfo\": \"" << userinfo << "\""
     << ", \"param\": {";
  bool first = true;
  for (const auto& pair : params) {
    if (first) {
      first = false;
    } else {
      os << ", ";
    }
    os << "\"" << pair.first << "\": \"" << pair.second << "\"";
  }
  os << "}";
}

const std::string& HttpUrlEntity::GetParam(const std::string& key, bool* exist/* = NULL*/) const {
  static const std::string k_empty_string;
  auto it = params.find(key);
  if (it != params.end()) {
    if (exist) *exist = true;
    return it->second;
  }
    if (exist) *exist = false;
  return k_empty_string;
}

void HttpUrlEntity::BuildFromSlice(const SliceHttpUrlEntity& entity) {
  schema.assign(entity.schema.data(), entity.schema.size());
  host.assign(entity.host.data(), entity.host.size());
  port.assign(entity.port.data(), entity.port.size());
  path.assign(entity.path.data(), entity.path.size());
  query.assign(entity.query.data(), entity.query.size());
  fragment.assign(entity.fragment.data(), entity.fragment.size());
  userinfo.assign(entity.userinfo.data(), entity.userinfo.size());
  params.clear();
  for (const auto& pair : entity.params) {
    params.insert(std::make_pair(
        pair.first.str(), 
        strutil::UrlDecode(pair.second.data(), pair.second.size())));
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
    : Protocol(ProtocolType::kHttpProtocol, true) {
}

ProtocolMessage* HttpProtocol::Decode(
    IOBuffer* buffer, 
    int* result,
    ProtocolMessage* /* hint TODO this should be useful*/) {
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
  static const std::string field_content_type = "Content-Type";

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

  // add Connection if not exist
  if (http_message->header.find(field_connection) == http_message->header.end()) {
    if (http_message->keep_alive) {
      WRITE_BUFFER_CONST("Connection: Keep-Alive\r\n");
    } else {
      WRITE_BUFFER_CONST("Connection: Close\r\n");
    }
  }

  // add sequence id if not exist
  if (http_message->header.find(field_x_seq_id) == http_message->header.end()) {
    WRITE_BUFFER_CONST("X-SEQ-ID: ");
    buffer->WriteString(boost::lexical_cast<std::string>(http_message->sequence_id));
    WRITE_BUFFER_CONST("\r\n");
  }

  if (http_message->direction == ProtocolMessage::kOutgoingResponse) {
    //add Set-Cookie
    for(const auto& cookie : http_message->set_cookies) {
      WRITE_BUFFER_CONST("Set-Cookie: ");
      buffer->WriteString(cookie);
      WRITE_BUFFER_CONST("\r\n");
    }
  }

  // add Content-Type if not exist
  if (http_message->header.find(field_content_type) == http_message->header.end()) {
    WRITE_BUFFER_CONST("Content-Type: ");
    WRITE_BUFFER_CONST("text/html");
    WRITE_BUFFER_CONST("\r\n");
  } 

  // add Content-Length if not exist
  if (http_message->header.find(field_content_length) == http_message->header.end()) {
    if (http_message->gzip && http_message->body.size() >= 128) {
      std::string gzip = Gzip(http_message->body);

      WRITE_BUFFER_CONST("Content-Length: ");
      buffer->WriteString(boost::lexical_cast<std::string>(gzip.size()));
      WRITE_BUFFER_CONST("\r\n");

      WRITE_BUFFER_CONST("Content-Encoding: ");
      WRITE_BUFFER_CONST("gzip");
      WRITE_BUFFER_CONST("\r\n");

      WRITE_BUFFER_CONST("\r\n");
      buffer->WriteString(gzip);
    } else {
      WRITE_BUFFER_CONST("Content-Length: ");
      buffer->WriteString(boost::lexical_cast<std::string>(http_message->body.size()));
      WRITE_BUFFER_CONST("\r\n");
      WRITE_BUFFER_CONST("\r\n");
      buffer->WriteString(http_message->body);
    }
  } else {
    WRITE_BUFFER_CONST("\r\n");
    buffer->WriteString(http_message->body);
  } 

  MI_LOG_TRACE(logger, "HttpProtocol::EncodeMessage: " << buffer->GetCStyle());
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

int HttpProtocol::ParseUrl(const Slice& url, SliceHttpUrlEntity* entity) {
  struct http_parser_url u;
  int ret = http_parser_parse_url(url.data(), url.size(), 0, &u);
  if (ret != 0) {
    return ret;
  }
  const char* ptr = url.data();

  if (u.field_set & (1 << UF_SCHEMA)) {
    entity->schema.set(
        ptr + u.field_data[UF_SCHEMA].off,
        u.field_data[UF_SCHEMA].len);
  }

  if (u.field_set & (1 << UF_HOST)) {
    entity->host.set(
        ptr + u.field_data[UF_HOST].off,
        u.field_data[UF_HOST].len);
  }

  if (u.field_set & (1 << UF_PORT)) {
    entity->port.set(
        ptr + u.field_data[UF_PORT].off,
        u.field_data[UF_PORT].len);
  }

  if (u.field_set & (1 << UF_PATH)) {
    entity->path.set(
        ptr + u.field_data[UF_PATH].off,
        u.field_data[UF_PATH].len);
  }

  if (u.field_set & (1 << UF_QUERY)) {
    entity->query.set(
        ptr + u.field_data[UF_QUERY].off,
        u.field_data[UF_QUERY].len);
  }

  if (u.field_set & (1 << UF_FRAGMENT)) {
    entity->fragment.set(
        ptr + u.field_data[UF_FRAGMENT].off,
        u.field_data[UF_FRAGMENT].len);
  }

  if (u.field_set & (1 << UF_USERINFO)) {
    entity->userinfo.set(
        ptr + u.field_data[UF_USERINFO].off,
        u.field_data[UF_USERINFO].len);
  }

  return ParseParam(entity->query, &entity->params);
}

int HttpProtocol::ParseParam(const Slice& query, std::map<Slice, Slice>* param) {
  const static uint8_t KEY = 0;
  const static uint8_t VALUE = 1;

  const char* begin = NULL;
  const char* current = query.data();
  uint8_t flag = KEY;

  Slice key;

  for (; current != query.data() + query.size(); ++current) {
    if (!begin) begin = current;
    if (flag == KEY && *current == '=') {
      key.set(begin, current - begin);
      begin = NULL;
      flag = 1 - flag; // flip
    } else if (flag == VALUE && *current == '&') {
      param->insert(std::make_pair(key, Slice(begin, current - begin)));
      begin = NULL;
      flag = 1 - flag;
    }
  }

  if (flag == VALUE) {
    if (begin) {
      param->insert(std::make_pair(key, Slice(begin, current - begin)));
    } else {
      param->insert(std::make_pair(key, Slice()));
    }
  } else if (begin != NULL) {
    return -1;
  }

  return 0;
}

int HttpProtocol::ParseCookie(const Slice& cookies, std::map<Slice, Slice>* cookie) {
  const static uint8_t KEY = 0;
  const static uint8_t VALUE = 1;

  const char* begin = NULL;
  const char* current = cookies.data();
  uint8_t flag = KEY;

  Slice key;

  for (; current != cookies.data() + cookies.size(); ++current) {
    if (!begin) begin = current;
    if (flag == KEY && *current == '=') {
      key.set(begin, current - begin);
      begin = NULL;
      flag = 1 - flag; // flip
    } else if (flag == VALUE && *current == ';') {
      cookie->insert(std::make_pair(key.strip(" "), Slice(begin, current - begin)));
      begin = NULL;
      flag = 1 - flag;
    }
  }

  if (flag == VALUE) {
    if (begin) {
      cookie->insert(std::make_pair(key.strip(" "), Slice(begin, current - begin)));
    } else {
      cookie->insert(std::make_pair(key.strip(" "), Slice()));
    }
  } else if (begin != NULL) {
    return -1;
  }

  return 0;
}

void HttpProtocol::Reset() {
  parser_.Reset();
}

// @see php_zlib.h, guess the maximal buffer size of deflate
#define GZIP_BUFSIZ_GUESS(in_len) \
   (((size_t) ((double) in_len * (double) 1.015)) + 10 + 8 + 4 + 1)


static voidpf zalloc_cb(voidpf opaque, uInt items, uInt size) {
  (void) opaque;
  /* not a typo, keep it calloc() */
  return (voidpf) calloc(items, size);
}

static void zfree_cb(voidpf opaque, voidpf ptr) {
  (void) opaque;
  free(ptr);
}

std::string HttpProtocol::Gzip(const std::string& raw) {
  std::string result(GZIP_BUFSIZ_GUESS(raw.size()), 0); 

  z_stream z;
  memset(&z, 0, sizeof(z));
  z.zalloc = &zalloc_cb;
  z.zfree = &zfree_cb;
  if (deflateInit2(
        &z, -1, Z_DEFLATED,
        MAX_WBITS + 16, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY) != Z_OK) {
    LOG_FATAL(logger, "HttpProtocol::Gzip fail!");
    return result;
  }

  z.next_in = (Bytef *)raw.data();
  z.avail_in = raw.size();
  z.next_out = (Bytef *) strutil::GetMutableBuffer(result);
  z.avail_out = result.size();

  if (deflate(&z, Z_FINISH) != Z_STREAM_END || deflateEnd(&z) != Z_OK) {
    LOG_FATAL(logger, "HttpProtocol::Gzip fail!");
    return result;
  }

  result.resize(z.total_out);
  return result;
}

std::ostream& operator << (std::ostream& os, const SliceHttpUrlEntity& entity) {
  entity.Dump(os);
  return os;
}

std::ostream& operator << (std::ostream& os, const HttpUrlEntity& entity) {
  entity.Dump(os);
  return os;
}

} //namespace ade
