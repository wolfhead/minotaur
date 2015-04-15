#ifndef _MINOTAUR_NET_CLIENT_CHANNEL_H_
#define _MINOTAUR_NET_CLIENT_CHANNEL_H_
/**
 * @file client_channel.h
 * @author Wolfhead
 */

#include "channel.h"
#include "socket_op.h"
#include "async_sequence_keeper.h"

namespace minotaur {

class ClientChannel : public Channel {
 public:
  enum {
    kConnecting = 0,
    kConnected,
    kBroken,
    kStopping,
  };

  ClientChannel(
      IOService* io_service, 
      uint32_t timeout_msec,
      uint32_t heartbeat_msec);

  ~ClientChannel();

  virtual int Start();

  virtual int Stop();

  inline int GetStatus() const {return status_;}

  inline void SetStatus(int status) {status_ = status;}

 private:
  LOGGER_CLASS_DECL(logger);

  int StartTimeoutTimer();

  int StartHeartBeatTimer();

  virtual void OnWrite();

  virtual void OnClose();

  virtual void OnActiveClose();

  virtual void OnConnect();

  virtual void OnTimeout();

  virtual void OnHeartBeat();

  virtual int EncodeMessage(ProtocolMessage* message);

  virtual void OnDecodeMessage(ProtocolMessage* message);

  void BreakChannel();

  int TryConnect();

  void CancelTimer();

  struct sockaddr_in sock_addr_;
  int status_;
  uint32_t timeout_ms_;
  uint32_t heartbeat_ms_;
  uint64_t reconnect_timer_;
  uint64_t timeout_timer_;
  uint64_t heartbeat_timer_;
  AsyncSequenceKeeper sequence_keeper_;
};

} //namespace socket

#endif //_MINOTAUR_NET_CLIENT_CHANNEL_H_
