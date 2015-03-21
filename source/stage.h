#ifndef _MINOTAUR_STAGE_H_
#define _MINOTAUR_STAGE_H_
/**
 * @file stage.h
 * @author Wolfhead
 */
#include <boost/thread.hpp>
#include "queue/sequencer.hpp"
#include "queue/fifo.h"
#include "message_queue.h"
#include "message.h"

namespace minotaur {

struct StageData {
  typedef typename queue::MPSCQueue<
    EventMessage, 
    queue::ConditionVariableStrategy<0, 16> > MessageQueueType;

  typedef typename queue::MPSCQueue<
    EventMessage,
    queue::NoWaitStrategy> PriorityMessageQueueType;

  MessageQueueType* queue;
  PriorityMessageQueueType* pri_queue;
  void* handler;
  bool running;
  boost::thread* thread;
  uint16_t handler_id;
};

template<typename HandlerType>
class Stage {
 public:
  typedef typename StageData::MessageQueueType MessageQueueType;

  typedef typename StageData::PriorityMessageQueueType PriorityMessageQueueType;

  Stage(
      HandlerType* prototype,
      uint32_t worker_count, 
      uint32_t queue_size);

  virtual ~Stage();

  int Start();
  int Wait();
  int Stop();
  bool Send(const EventMessage& message);
  bool SendPriority(const EventMessage& message);

 private:

  int Destroy();
  
  HandlerType* prototype_;
  uint32_t worker_count_;
  uint32_t queue_size_;
  std::vector<StageData*> data_;
};

} //namespace minotaur

#include "stage.impl.h"

#endif // _MINOTAUR_STAGE_H_
