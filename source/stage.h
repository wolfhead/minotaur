#ifndef _MINOTAUR_STAGE_H_
#define _MINOTAUR_STAGE_H_
/**
 * @file stage.h
 * @author Wolfhead
 */
#include <boost/thread.hpp>
#include "message_queue.h"

namespace minotaur {

template <typename Handler>
class StageWorker {
 public:
  typedef typename Handler::MessageType MessageType;
  typedef MessageQueue<MessageType> MessageQueueType;

  StageWorker(); 

  ~StageWorker(); 

  void SetHandler(Handler* handler, bool own);
  Handler* GetHandler() {return handler_;}

  void SetQueue(MessageQueueType* queue, bool own);
  MessageQueueType* GetMessageQueue() {return queue_;}

  int Start();
  void Stop();
  void Join();

 private:

  void Run();

  bool running_;
  boost::thread* thread_;

  Handler* handler_;
  bool own_handler_;

  MessageQueueType* queue_;
  bool own_queue_;
};

template <typename Handler>
class Stage {
 public:
  typedef typename Handler::MessageType MessageType;
  typedef MessageQueue<MessageType> MessageQueueType;
  typedef StageWorker<Handler> StageWorkerType;

  Stage(uint32_t worker_count, uint32_t queue_size);

  ~Stage();

  int Start();
  void Wait();
  void Stop();
  bool Send(const MessageType& message);

 private:
  void BuildWorker();
  void BindQueue();
  void BindHandler();

  int StartWorker();
  void DestroyWorker();

  uint32_t worker_count_;
  uint32_t queue_size_;

  MessageQueueType* queue_;
  Handler* handler_;
  StageWorkerType* worker_;
};

} //namespace minotaur

#include "stage_impl.h"

#endif // _MINOTAUR_STAGE_H_
