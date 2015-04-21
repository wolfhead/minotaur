#ifndef MINOTAUR_STDIO_CHANNEL_H_
#define MINOTAUR_STDIO_CHANNEL_H_
/**
 * @file stdio_channel.h
 * @author Wolfhead
 */

#include "channel.h"

namespace minotaur {

class StdioChannel : public Channel {
 public:
  StdioChannel(IOService* io_service);

  ~StdioChannel();

  virtual int Start();

  virtual int Stop();
};


} //namesapce minotaur

#endif//MINOTAUR_STDIO_CHANNEL_H_ 
