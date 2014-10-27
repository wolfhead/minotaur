#ifndef _MINOTAUR_MATRIX_ITEM_MAP_H_
#define _MINOTAUR_MATRIX_ITEM_MAP_H_
/**
 * @file matrix_item_map.h
 * @author Wolfhead
 */
#include <stdint.h>
#include <sys/syscall.h>
#include <string>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>

namespace minotaur { namespace matrix {

class MatrixItem {
 public:
  MatrixItem(const std::string& _name)
      : item_name(_name) {
  }
  std::string item_name;
  uint64_t time_start_usec;
};

class MatrixItemMap {
 public:
  typedef boost::unordered_map<uint32_t, MatrixItem> ItemMap;
  typedef boost::mutex LockType;

  MatrixItemMap();
  ~MatrixItemMap();
  uint64_t Create(const std::string& name);
  bool Get(uint64_t id, MatrixItem* item);
  void ClearExpire(uint32_t usec_timeout);

 private:
  static const uint16_t bucket_count = 1024;
  static inline uint64_t BuildMaxtrixItemId(
      uint16_t bucket_id,
      uint32_t local_id) {
    return (uint64_t)bucket_id << 48 | local_id;
  }
  static inline uint16_t GetBucketIdFromMaxtrixItemId(uint64_t id) {
    return id >> 32 & 0xFFFF;
  }
  static inline uint16_t GetLocalIdFromMaxtrixItemId(uint64_t id) {
    return id & 0xFFFFFFFF;
  }

  inline uint16_t GetBucket() {
    return syscall(SYS_gettid) & (bucket_count - 1);
  }

  inline uint32_t GetLocalId() {
    if (!thread_seq_.get()) {
      thread_seq_.reset(new uint32_t);
    }

    return ++(*thread_seq_.get());
  }

  void ClearOneBucket(
      uint16_t bucket_id, 
      uint64_t current_usec, 
      uint32_t usec_timeout);
  

  ItemMap* maps_;
  LockType* locks_; 
  boost::thread_specific_ptr<uint32_t> thread_seq_;
};


} //namespace matrix
} //namespace minotaur

#endif // _MINOTAUR_MATRIX_ITEM_MAP_H_
