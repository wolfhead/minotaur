#ifndef MINOTAUR_STDIO_CHANNEL_H_
#define MINOTAUR_STDIO_CHANNEL_H_
/**
 * @file stdio_channel.h
 * @author Wolfhead
 */

#include "io_descriptor.h"

namespace ade {

class Service;

class StdioChannel : public IODescriptor {
 public:
  StdioChannel(IOService* io_service, Service* service);

  ~StdioChannel();

  virtual int Start();

  virtual int Stop();

 protected:
  virtual void Close();

  virtual void OnClose();
 private:
  LOGGER_CLASS_DECL(logger);
};


} //namesapce ade

#endif//MINOTAUR_STDIO_CHANNEL_H_ 
