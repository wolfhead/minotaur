/**
 * @file matrix_collector.cpp
 * @author Wolfhead
 */
#include "matrix_collector.h"
#include <sys/prctl.h>
#include "matrix_stat_map.h"
#include "matrix.h"

namespace ade { namespace matrix {

LOGGER_CLASS_IMPL(logger, MatrixCollector);
LOGGER_CLASS_IMPL_NAME(collector_logger, MatrixCollector, "ade.matrix");
LOGGER_CLASS_IMPL_NAME(collector_logger_simple, MatrixCollector, "ade.matrix.simple");

MatrixCollector::MatrixCollector(uint32_t bucket_count, uint32_t queue_size)
    : bucket_count_(bucket_count)
    , queue_size_(queue_size)
    , thread_(NULL) 
    , running_(false) 
    , stat_map_(new MatrixStatMap(time(NULL))) {
  queue_.resize(bucket_count_);
  for (auto& queue : queue_) {
    queue = new QueueType(queue_size_);    
  }
}

MatrixCollector::~MatrixCollector() {
  Stop();

  for (auto& queue : queue_) {
    delete queue;
  }
  queue_.clear();
}

int MatrixCollector::Start() {
  if (thread_) {
    return 0;
  }

  running_ = true;
  thread_ = new boost::thread(boost::bind(&MatrixCollector::Run, this));
  return 0;
}

int MatrixCollector::Stop() {
  if (!thread_) {
    return 0;
  }

  running_ = false;
  thread_->join();
  delete thread_;
  thread_ = NULL;
  return 0;
}

void MatrixCollector::Run() {
  prctl(PR_SET_NAME, "matrix");

  LOG_INFO(logger, "MatrixCollector Running");
  MatrixStatMapPtr stat_map(new MatrixStatMap(time(NULL)));

  while (running_) {
    if (time(NULL) - stat_map->GetStartTime() > 30) {
      stat_map->Freeze();
      MI_LOG_INFO(collector_logger, "MatrixCollector:\n" << *stat_map);
      MI_LOG_INFO(collector_logger_simple, "MatrixCollectorSimple:\n" 
          << stat_map->ToStringSimple());

      boost::atomic_store(&stat_map_, stat_map);
      stat_map = MatrixStatMapPtr(new MatrixStatMap(time(NULL)));
      stat_map->Inherit(*stat_map_);
    }

    for (uint32_t i = 0; i != bucket_count_; ++i) {
      QueueType* queue = queue_[i];
      ProcessQueue(queue, stat_map);
    }
    sleep(1);
  }

  LOG_INFO(logger, "MatrixCollector Exit");
}

void MatrixCollector::ProcessQueue(QueueType* queue, MatrixStatMapPtr stat_map) {
  const MatrixItem* item;
  while (queue->Pop(&item)) {
    switch (item->operation) {
      case Matrix::kSet : stat_map->Set(item->name, item->val, item->persistent); break;
      case Matrix::kAdd : stat_map->Add(item->name, item->val, item->persistent); break;
      case Matrix::kSub : stat_map->Sub(item->name, item->val, item->persistent); break;
      case Matrix::kReset : stat_map->Reset(item->name); break;
      case Matrix::kTimeDistribute : {
          if (item->result.empty()) {
            stat_map->TimeDistrubute(item->name, item->val, item->persistent);
          } else {
            stat_map->TimeDistrubute(item->name, item->result, item->val, item->persistent);
          }
          break;
      }
      default: LOG_WARN(logger, "MatrixCollector::ProcessEvent unknown operation:" << *item); break;
    }
    delete item;
  }
}

} //namespace matrix
} //namespace ade
