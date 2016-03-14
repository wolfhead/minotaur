#ifndef _MINOTAUR_MATRIX_ITEM_MAP_H_
#define _MINOTAUR_MATRIX_ITEM_MAP_H_
/**
 * @file matrix_item_map.h
 * @author Wolfhead
 */
#include <stdint.h>
#include <string>
#include <vector>
#include <unordered_map>
#include "matrix_item.h"
#include "../common/logger.h"
#include "../common/spin_lock.h"

namespace ade { namespace matrix {

class MatrixBucketItemMap {
 public:
  typedef std::unordered_map<uint64_t, MatrixItem*> ItemMap;
  typedef spinlock LockType;

  int GenerateToken(uint64_t token, const std::string& name);
  int FetchToken(uint64_t token, MatrixItem** item);

 private:
  ItemMap map_;
  LockType lock_;
};

class MatrixItemMap {
 public:
  MatrixItemMap(uint32_t bucket_count);
  ~MatrixItemMap();
  uint64_t GenerateToken(const std::string& name);
  int FetchToken(uint64_t token_id, MatrixItem** item);

  inline MatrixBucketItemMap& GetBucket(uint64_t token) {
    return buckets_[token & (bucket_count_ - 1)];
  }

 private:
  LOGGER_CLASS_DECL(logger);
  
  uint32_t bucket_count_;
  MatrixBucketItemMap* buckets_;
  static thread_local uint32_t token_id_;
};


} //namespace matrix
} //namespace ade

#endif // _MINOTAUR_MATRIX_ITEM_MAP_H_
