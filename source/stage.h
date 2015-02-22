#ifndef _MINOTAUR_STAGE_H_
#define _MINOTAUR_STAGE_H_
/**
 * @file stage.h
 * @author Wolfhead
 */
#include <boost/thread.hpp>
#include "queue/sequencer.hpp"
#include "message_queue.h"

namespace minotaur {

template<typename MessageType, bool shared>
struct QueueHelper {
};

template<typename MessageType>
struct QueueHelper<MessageType, true> {
  typedef typename queue::MPMCQueue<
    MessageType,
    queue::ConditionVariableStrategy<0, 256> > MessageQueueType;

  typedef typename queue::MPMCQueue<
    MessageType,
    queue::NoWaitStrategy> PriorityMessageQueueType;
};

template<typename MessageType>
struct QueueHelper<MessageType, false> {
  typedef typename queue::MPSCQueue<
    MessageType, 
    queue::ConditionVariableStrategy<0, 256> > MessageQueueType;

  typedef typename queue::MPSCQueue<
    MessageType,
    queue::NoWaitStrategy> PriorityMessageQueueType;
  
  //typedef typename queue::MPSCQueue<
  //  MessageType, 
  //  queue::BusyLoopStrategy> MessageQueueType;
};

template <typename Handler>
class StageWorker {
 public:
  typedef typename Handler::MessageType MessageType;
  typedef typename QueueHelper<
    MessageType, 
    Handler::share_queue>::MessageQueueType MessageQueueType;
  typedef typename QueueHelper<
    MessageType, 
    Handler::share_queue>::PriorityMessageQueueType PriorityMessageQueueType;

  StageWorker(); 

  ~StageWorker(); 

  void SetHandler(Handler* handler, bool own);
  Handler* GetHandler() {return handler_;}

  void SetQueue(
      MessageQueueType* queue, 
      PriorityMessageQueueType* pri_queue,
      bool own);
  MessageQueueType* GetMessageQueue() {return queue_;}
  PriorityMessageQueueType* GetPriorityMessageQueue() {return pri_queue_;}

  void SetStageName(const std::string& name) {stage_name_ = name;}

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
  PriorityMessageQueueType* pri_queue_;
  bool own_queue_;

  std::string stage_name_;
};

template <typename HandlerFactory>
class Stage {
 public:
  typedef typename HandlerFactory::Handler Handler; 
  typedef StageWorker<Handler> StageWorkerType;
  typedef typename Handler::MessageType MessageType;
  typedef typename QueueHelper<
    MessageType, 
    Handler::share_queue>::MessageQueueType MessageQueueType;
  typedef typename QueueHelper<
    MessageType, 
    Handler::share_queue>::PriorityMessageQueueType PriorityMessageQueueType;


  Stage(
      HandlerFactory* factory, 
      uint32_t worker_count, 
      uint32_t queue_size);

  ~Stage();

  int Start();
  int Wait();
  int Stop();
  bool Send(const MessageType& message);
  bool SendPriority(const MessageType& message);

  void SetStageName(const std::string& name) {stage_name_ = name;}

 private:
  int BuildWorker();
  int BindQueue();
  int BindHandler();

  int StartWorker();
  void DestroyWorker();

  uint32_t worker_count_;
  uint32_t queue_size_;

  HandlerFactory* factory_;
  MessageQueueType* queue_;
  PriorityMessageQueueType* pri_queue_;
  Handler* handler_;
  StageWorkerType* worker_;
  std::string stage_name_;
};

} //namespace minotaur

#include "stage_impl.h"

#endif // _MINOTAUR_STAGE_H_
