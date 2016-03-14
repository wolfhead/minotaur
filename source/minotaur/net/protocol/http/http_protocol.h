#ifndef _MINOTAUR_HTTP_PROTOCOL_H_
#define _MINOTAUR_HTTP_PROTOCOL_H_
/**
 * @file http_protocol.h
 * @author Wolfhead
 */
#include "../protocol.h"
#include <list>
#include "../../../common/logger.h"
#include "../../../common/slice.h"
#include "../../../3rd-party/http_parser/http_parser.h"

namespace ade {

class HttpMessage;

struct HttpProtocolData {
  HttpProtocolData();
  ~HttpProtocolData();
  void Reset();

  struct http_parser parser;
  std::list<ProtocolMessage*> messages;
  HttpMessage* current; 
  std::string field;
  std::string value;
  bool on_header_value;

  static struct http_parser_settings parser_setting;;
};

struct SliceHttpUrlEntity {
  Slice schema;
  Slice host;
  Slice port;
  Slice path;
  Slice query;
  Slice fragment;
  Slice userinfo; 
  std::map<Slice, Slice> params; //!! the value is not decoede
  void Dump(std::ostream& os) const;
};

struct HttpUrlEntity {
  HttpUrlEntity();
  HttpUrlEntity(const SliceHttpUrlEntity& entity);

  const std::string& GetParam(const std::string& key, bool* exist = NULL) const;

  void BuildFromSlice(const SliceHttpUrlEntity& entity);
  void Dump(std::ostream& os) const;

  std::string schema;
  std::string host;
  std::string port;
  std::string path;
  std::string query;
  std::string fragment;
  std::string userinfo;
  std::map<std::string, std::string> params;
};

class HttpProtocol : public Protocol {
 public:
  enum {
#define XX(num, name, string) kMethod##name = num,
    HTTP_METHOD_MAP(XX)
#undef XX
  }; 

  static const std::string& GetMethodString(int method);
  static const std::string& GetStatusString(int status);

  HttpProtocol();
  
  virtual ProtocolMessage* Decode(
      IOBuffer* buffer, 
      int* result,
      ProtocolMessage* hint);

  virtual int Encode(
      IOBuffer* buffer,
      ProtocolMessage* message);

  virtual ProtocolMessage* HeartBeatRequest();

  virtual ProtocolMessage* HeartBeatResponse(ProtocolMessage* request);

  virtual Protocol* Clone() {return new HttpProtocol();}
  virtual void Destroy() {delete this;}

  virtual void Reset();

  static int ParseUrl(const Slice& url, SliceHttpUrlEntity* entity);

  static int ParseParam(const Slice& query, std::map<Slice, Slice>* param);

  static int ParseCookie(const Slice& cookies, std::map<Slice, Slice>* cookie);

  static std::string Gzip(const std::string& raw);

 private:
  LOGGER_CLASS_DECL(logger);

  int EncodeRequest(IOBuffer* buffer, ProtocolMessage* message);

  int EncodeResponse(IOBuffer* buffer, ProtocolMessage* message);

  HttpProtocolData parser_;;
};

std::ostream& operator << (std::ostream& os, const SliceHttpUrlEntity& entity);
std::ostream& operator << (std::ostream& os, const HttpUrlEntity& entity);

} //namespace ade

#endif //_MINOTAUR_HTTP_PROTOCOL_H_
