#ifndef _MINOTAUR_MATRIX_COLLECTOR_H_
#define _MINOTAUR_MATRIX_COLLECTOR_H_

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include "../common/logger.h"
#include "../common/thread_id.h"
#include "../queue/sequencer.hpp"
#include "matrix_item_map.h"

namespace ade { namespace matrix {

class MatrixStatMap;

class MatrixCollector {
 public:
  typedef typename queue::MPSCQueue<
    const MatrixItem*,
    queue::NoWaitStrategy> QueueType;
  typedef boost::shared_ptr<MatrixStatMap> MatrixStatMapPtr;

  MatrixCollector(uint32_t bucket_count, uint32_t queue_size);
  ~MatrixCollector();

  int Start();
  int Stop();

  inline int Send(const MatrixItem* item) {
    return GetQueue()->Push(item) ? 0 : -1;
  }

  inline MatrixStatMapPtr GetMatrixStatMap() {
    return stat_map_;
  }

 private:
  LOGGER_CLASS_DECL(logger);
  LOGGER_CLASS_DECL(collector_logger);
  LOGGER_CLASS_DECL(collector_logger_simple);

  inline QueueType* GetQueue() {
    return queue_.at(ThreadId::Get() & (bucket_count_ - 1));
  }

  void Run();
  void ProcessQueue(QueueType* queue, MatrixStatMapPtr stat_map);

  std::vector<QueueType*> queue_;
  uint32_t bucket_count_;
  uint32_t queue_size_;
  boost::thread* thread_;
  bool running_;

  MatrixStatMapPtr stat_map_;
};

} //namespace matrix
} //namesapce ade

#endif // _MINOTAUR_MATRIX_COLLECTOR_H_
