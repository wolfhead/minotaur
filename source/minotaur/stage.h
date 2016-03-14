#ifndef _MINOTAUR_STAGE_H_
#define _MINOTAUR_STAGE_H_
/**
 * @file stage.h
 * @author Wolfhead
 */
#include <boost/thread.hpp>
#include "queue/sem_fifo.h"
#include "queue/sequencer.hpp"
#include "message.h"

namespace ade {

template<typename Handler>
struct StageData {
  typedef typename Handler::MessageType MessageType;

  typedef queue::SemFifo<MessageType> MessageQueueType;

  //typedef queue::MPSCQueue<MessageType, queue::ConditionVariableStrategy<4> > MessageQueueType;

  MessageQueueType* queue;
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

  Stage(
      HandlerType* prototype,
      uint32_t worker_count, 
      uint32_t queue_size,
      bool is_shared_queue,
      bool is_shared_handler);

  virtual ~Stage();

  int Start();
  int Wait();
  int Stop();
  bool Send(const MessageType& message);
  bool SendPriority(const MessageType& message);

  void Dump(std::ostream& os) const;
  std::string ToString() const;

 private:

  int Destroy();
  
  HandlerType* prototype_;
  uint32_t worker_count_;
  uint32_t queue_size_;
  std::vector<StageDataType*> data_;

  bool is_shared_queue_;
  bool is_shared_handler_;
  MessageQueueType* shared_queue_;
};

} //namespace ade

#include "stage.impl.h"

#endif // _MINOTAUR_STAGE_H_
