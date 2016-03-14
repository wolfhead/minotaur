#ifndef _MINOTAUR_NET_CLIENT_CHANNEL_H_
#define _MINOTAUR_NET_CLIENT_CHANNEL_H_
/**
 * @file client_channel.h
 * @author Wolfhead
 */

#include "channel.h"
#include "socket_op.h"

namespace ade {

class ClientChannel : public Channel {
 public:
  enum {
    kConnecting = 0,
    kConnected,
    kBroken,
    kStopping,
  };

  static const std::string& GetStatusString(int status) {
    static const std::string str[] = {
      "connecting",
      "connected",
      "broken",
      "stopping",
    };
    static const std::string unknown = "unknown";
    if (status < 0 || status >= (int)(sizeof(str) / sizeof(std::string))) {
      return unknown;
    }
    return str[status];
  }

  ClientChannel(
      IOService* io_service, 
      uint32_t timeout_msec,
      uint32_t heartbeat_msec);

  ~ClientChannel();

  virtual int Start();

  virtual int Stop();

  inline int GetStatus() const {return status_;}

  inline void SetStatus(int status) {status_ = status;}

 protected:
  LOGGER_CLASS_DECL(logger);

  int StartReconnectTimer();

  int StartTimeoutTimer();

  int StartHeartBeatTimer();

  virtual void OnWrite();

  virtual void OnClose();

  virtual void OnActiveClose();

  virtual void OnConnect();

  virtual void OnHeartBeat();

  void BreakChannel();

  int TryConnect();

  void CancelTimer();

  virtual void ResetLocal();

  void DoSendBack(ProtocolMessage* message, int status);

  virtual void OnTimeout() = 0;

  virtual void PurgeSequenceKeeper() = 0;

  struct sockaddr_in sock_addr_;
  int status_;
  uint32_t timeout_ms_;
  uint32_t heartbeat_ms_;
  uint64_t reconnect_timer_;
  uint64_t timeout_timer_;
  uint64_t heartbeat_timer_;
};

} //namespace socket

#endif //_MINOTAUR_NET_CLIENT_CHANNEL_H_
