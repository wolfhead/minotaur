#ifndef _MINOTAUR_HTTP_PROTOCOL_H_
#define _MINOTAUR_HTTP_PROTOCOL_H_
/**
 * @file http_protocol.h
 * @author Wolfhead
 */
#include "../protocol.h"
#include "http_parser.h"
#include <list>
#include "../../../common/logger.h"

namespace minotaur {

class HttpMessage;

struct HttpProtocolData {
  HttpProtocolData() {
    http_parser_init((http_parser*)this, HTTP_REQUEST);
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
  HttpProtocol();
  
  virtual ProtocolMessage* Decode(
      IODescriptor* descriptor, 
      IOBuffer* buffer, 
      int* result);

  virtual bool Encode(
      IODescriptor* descriptor,
      IOBuffer* buffer,
      ProtocolMessage* message);

  virtual Protocol* Clone() {return new HttpProtocol();}
  virtual void Destroy() {delete this;}

 private:
  LOGGER_CLASS_DECL(logger);
  HttpProtocolData parser_;;
};

} //namespace minotaur

#endif //_MINOTAUR_HTTP_PROTOCOL_H_
