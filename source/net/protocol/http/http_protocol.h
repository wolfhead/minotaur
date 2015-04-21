#ifndef _MINOTAUR_HTTP_PROTOCOL_H_
#define _MINOTAUR_HTTP_PROTOCOL_H_
/**
 * @file http_protocol.h
 * @author Wolfhead
 */
#include "../protocol.h"
#include <list>
#include "../../../common/logger.h"
#include "../../../3rd-party/http_parser/http_parser.h"

namespace minotaur {

class HttpMessage;

struct HttpProtocolData {
  HttpProtocolData() {
    http_parser_init((http_parser*)this, HTTP_BOTH);
    current = NULL;
    on_header_value = false;
  }

  ~HttpProtocolData();

  struct http_parser parser;
  std::list<ProtocolMessage*> messages;
  HttpMessage* current; 
  std::string field;
  std::string value;
  bool on_header_value;

  static struct http_parser_settings parser_setting;;
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
      int* result);

  virtual int Encode(
      IOBuffer* buffer,
      ProtocolMessage* message);

  virtual ProtocolMessage* HeartBeatRequest();

  virtual ProtocolMessage* HeartBeatResponse(ProtocolMessage* request);

  virtual Protocol* Clone() {return new HttpProtocol();}
  virtual void Destroy() {delete this;}

 private:
  LOGGER_CLASS_DECL(logger);

  int EncodeRequest(IOBuffer* buffer, ProtocolMessage* message);

  int EncodeResponse(IOBuffer* buffer, ProtocolMessage* message);

  HttpProtocolData parser_;;
};

} //namespace minotaur

#endif //_MINOTAUR_HTTP_PROTOCOL_H_
