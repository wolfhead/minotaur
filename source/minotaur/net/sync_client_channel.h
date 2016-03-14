#ifndef _MINOTAUR_NET_SYNC_CLIENT_CHANNEL_H_
#define _MINOTAUR_NET_SYNC_CLIENT_CHANNEL_H_
/**
 * @file sync_client_channel.h
 * @author Wolfhead
 */
#include "client_channel.h"
#include "sync_sequence_keeper.h"

namespace ade {

class SyncClientChannel : public ClientChannel {
 public:
  SyncClientChannel(
      IOService* io_service,
      uint32_t timeout_msec,
      uint32_t heartbeat_msec);

 private:
  LOGGER_CLASS_DECL(logger);

  virtual int EncodeMessage(ProtocolMessage* message);

  virtual void OnDecodeMessage(ProtocolMessage* message);

  virtual void OnTimeout();

  virtual void PurgeSequenceKeeper();

  virtual int DecodeMessage();

  void TryFireMessage();

  SyncSequenceKeeper sequence_keeper_;
};

} //namespace ade

#endif //_MINOTAUR_NET_SYNC_CLIENT_CHANNEL_H_
