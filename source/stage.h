#ifndef _MINOTAUR_STAGE_H_
#define _MINOTAUR_STAGE_H_
/**
 * @file stage.h
 * @author Wolfhead
 */
#include <boost/thread.hpp>
#include "queue/sequencer.hpp"
#include "queue/fifo.h"
#include "message.h"

namespace minotaur {

template<typename Handler>
struct StageData {
  typedef typename Handler::MessageType MessageType;

  typedef typename queue::MPSCQueue<
    MessageType, 
    queue::ConditionVariableStrategy<0, 16> > MessageQueueType;

  typedef typename queue::MPSCQueue<
    MessageType,
    queue::NoWaitStrategy> PriorityMessageQueueType;

  MessageQueueType* queue;
  PriorityMessageQueueType* pri_queue;
  Handler* handler;
  bool running;
  boost::thread* thread;
  uint16_t handler_id;
};

template<typename HandlerType>
class Stage {
 public:
  typedef StageData<HandlerType> StageDataType;
  typedef typename HandlerType::MessageType MessageType;
  typedef typename StageDataType::MessageQueueType MessageQueueType;
  typedef typename StageDataType::PriorityMessageQueueType PriorityMessageQueueType;

  Stage(
      HandlerType* prototype,
      uint32_t worker_count, 
      uint32_t queue_size);

  virtual ~Stage();

  int Start();
  int Wait();
  int Stop();
  bool Send(const MessageType& message);
  bool SendPriority(const MessageType& message);

 private:

  int Destroy();
  
  HandlerType* prototype_;
  uint32_t worker_count_;
  uint32_t queue_size_;
  std::vector<StageDataType*> data_;
};

} //namespace minotaur

#include "stage.impl.h"

#endif // _MINOTAUR_STAGE_H_
