#ifndef _MINOTAUR_STAGE_H_
#define _MINOTAUR_STAGE_H_
/**
 * @file stage.h
 * @author Wolfhead
 */
#include <boost/thread.hpp>
#include <queue/sequencer.hpp>
#include "message_queue.h"

namespace minotaur {


template<typename MessageType, bool shared>
struct QueueHelper {
};

template<typename MessageType>
struct QueueHelper<MessageType, true> {
  typedef typename queue::MPMCQueue<MessageType, queue::ConditionVariableStrategy> MessageQueueType;
};

template<typename MessageType>
struct QueueHelper<MessageType, false> {
  typedef typename queue::MPSCQueue<MessageType, queue::ConditionVariableStrategy> MessageQueueType;
};

template <typename Handler>
class StageWorker {
 public:
  typedef typename Handler::MessageType MessageType;
  typedef typename QueueHelper<MessageType, Handler::share_queue>::MessageQueueType MessageQueueType;

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
  typedef StageWorker<Handler> StageWorkerType;
  typedef typename Handler::MessageType MessageType;
  typedef typename QueueHelper<MessageType, Handler::share_queue>::MessageQueueType MessageQueueType;

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
