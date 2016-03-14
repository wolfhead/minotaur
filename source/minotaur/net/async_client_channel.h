#ifndef _MINOTAUR_NET_ASYNC_CLIENT_CHANNEL_H_
#define _MINOTAUR_NET_ASYNC_CLIENT_CHANNEL_H_
/**
 * @file async_client_channel.h
 * @author Wolfhead
 */
#include "client_channel.h"
#include "async_sequence_keeper.h"

namespace ade {

class AsyncClientChannel : public ClientChannel {
 public:
  AsyncClientChannel(
      IOService* io_service,
      uint32_t timeout_msec,
      uint32_t heartbeat_msec);

 private:
  LOGGER_CLASS_DECL(logger);

  virtual int EncodeMessage(ProtocolMessage* message);

  virtual void OnDecodeMessage(ProtocolMessage* message);

  virtual void OnTimeout();

  virtual void PurgeSequenceKeeper();

  AsyncSequenceKeeper sequence_keeper_;
};

} //namespace ade

#endif //_MINOTAUR_NET_ASYNC_CLIENT_CHANNEL_H_
