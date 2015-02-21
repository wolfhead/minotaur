#ifndef _MINOTAUR_STAGE_IMPL_H_
#define _MINOTAUR_STAGE_IMPL_H_
/**
 * @file stage_impl.h
 * @author Wolfhead
 */
#include "stage.h"
#include <sys/prctl.h>

namespace minotaur {

////////////////////////////////////////////////////////////////////////
// StageWorker

template <typename Handler>
StageWorker<Handler>::StageWorker() 
    : running_(false)
    , thread_(NULL)
    , handler_(NULL)
    , own_handler_(false)
    , queue_(NULL)
    , own_queue_(false) {
}

template <typename Handler>
StageWorker<Handler>::~StageWorker() {
  Stop();
  if (handler_ && own_handler_) {
    delete handler_;
    handler_ = NULL;
  }

  if (queue_ && own_queue_) {
    delete queue_;
    queue_ = NULL;
  }
}

template <typename Handler>
void StageWorker<Handler>::SetHandler(Handler* handler, bool own) {
  handler_ = handler;
  own_handler_ = own;
}

template <typename Handler>
void StageWorker<Handler>::SetQueue(MessageQueueType* queue, bool own) {
  queue_ = queue;
  own_queue_ = own;
}

template <typename Handler>
int StageWorker<Handler>::Start() {
  running_ = true;
  thread_ = new boost::thread(boost::bind(&StageWorker::Run, this));
  return 0;
}

template <typename Handler>
void StageWorker<Handler>::Stop() {
  running_ = false;
}

template <typename Handler>
void StageWorker<Handler>::Join() {
  if (thread_) {
    thread_->join();
  }
}

template <typename Handler>
void StageWorker<Handler>::Run() {
  prctl(PR_SET_NAME, stage_name_.c_str());

  MessageType message;
  while (running_) {
    if (!queue_->Pop(&message, 1)) {
      continue;
    }
    handler_->Handle(message);
  } 
}

////////////////////////////////////////////////////////////////////////
// Stage
template <typename HandlerFactory>
Stage<HandlerFactory>::Stage(
    HandlerFactory* factory, 
    uint32_t worker_count, 
    uint32_t queue_size) 
    : worker_count_(worker_count)
    , queue_size_(queue_size) 
    , factory_(factory)
    , queue_(NULL)
    , handler_(NULL) 
    , worker_(NULL) 
    , stage_name_("stage") {
}

template <typename HandlerFactory>
Stage<HandlerFactory>::~Stage() {
  delete factory_;
  DestroyWorker();
}

template <typename HandlerFactory>
int Stage<HandlerFactory>::Start() {
  BuildWorker();
  BindQueue();
  BindHandler();
  return StartWorker();
}

template <typename HandlerFactory>
void Stage<HandlerFactory>::Wait() {
  for (size_t i = 0; i != worker_count_; ++i) {
    worker_[i].Join();
  }
}

template <typename HandlerFactory>
void Stage<HandlerFactory>::Stop() {
  for (size_t i = 0; i != worker_count_; ++i) {
    worker_[i].Stop();
  }
}

template <typename HandlerFactory>
bool Stage<HandlerFactory>::Send(const MessageType& message) {
  MessageQueueType* queue = 
      worker_[Handler::HashMessage(message, worker_count_)].GetMessageQueue();
  if (!queue->Push(message)) {
    std::cout << "queue after length:" << queue->Size() << ", capacity:" << queue->Capacity() << std::endl;
    return false;
  }
  return true;
}

template <typename HandlerFactory>
void Stage<HandlerFactory>::BuildWorker() {
  worker_ = new StageWorkerType[worker_count_];    
}

template <typename HandlerFactory>
void Stage<HandlerFactory>::BindQueue() {
  if (Handler::share_queue) {
    queue_ = new MessageQueueType(queue_size_);
    for (size_t i = 0; i != worker_count_; ++i) {
      worker_[i].SetQueue(queue_, false);
    }
  } else {
    for (size_t i = 0; i != worker_count_; ++i) {
      worker_[i].SetQueue(new MessageQueueType(queue_size_), true);
    }      
  }  
}

template <typename HandlerFactory>
void Stage<HandlerFactory>::BindHandler() {
  if (Handler::share_handler) {
    handler_ = factory_->Create(this);
    for (size_t i = 0; i != worker_count_; ++i) {
      worker_[i].SetHandler(handler_, false);
    }
  } else {
    for (size_t i = 0; i != worker_count_; ++i) {
      worker_[i].SetHandler(factory_->Create(this), true);
    }      
  }  
}

template <typename HandlerFactory>
int Stage<HandlerFactory>::StartWorker() {
  for (size_t i = 0; i != worker_count_; ++i) {
    worker_[i].SetStageName(stage_name_);
    if (0 != worker_[i].Start()) {
      return -1;
    }
  }
  return 0;
}

template <typename HandlerFactory>
void Stage<HandlerFactory>::DestroyWorker() {
  if (worker_) {
    delete [] worker_;
    worker_ = NULL;
  }
  if (queue_) {
    delete queue_;
    queue_ = NULL;
  }
  if (handler_) {
    delete handler_;
    handler_ = NULL;
  }
}

} //namespace minotaur

#endif // _MINOTAUR_STAGE_IMPL_H_
