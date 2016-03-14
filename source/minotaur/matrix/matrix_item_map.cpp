/**
 * @file matrix_item_map.cpp
 * @author Wolfhead
 */
#include "matrix_item_map.h"
#include "matrix.h"
#include "../common/time_util.h"
#include "../common/thread_id.h"

namespace ade { namespace matrix {

LOGGER_CLASS_IMPL(logger, MatrixItemMap);
thread_local uint32_t MatrixItemMap::token_id_ = 0;

int MatrixBucketItemMap::GenerateToken(uint64_t token, const std::string& name) {
  lock_.lock();
  int ret = 
    map_.insert(std::make_pair(
          token, 
          new MatrixItem(Matrix::kTimeDistribute, name, Time::GetMicrosecond()))).second ?
    0 : -1;
  lock_.unlock();
  return ret;
}

int MatrixBucketItemMap::FetchToken(uint64_t token, MatrixItem** item) {
  lock_.lock();
  auto it = map_.find(token);
  if (it != map_.end()) {
    it->second->val = Time::GetMicrosecond() - it->second->val;
    *item = it->second;
    map_.erase(it);
    lock_.unlock();
    return 0;
  }
  lock_.unlock();
  return -1;
}

MatrixItemMap::MatrixItemMap(uint32_t bucket_count) 
    : bucket_count_(bucket_count) {
  buckets_ = new MatrixBucketItemMap[bucket_count];
}

MatrixItemMap::~MatrixItemMap() {
  delete [] buckets_;
}

uint64_t MatrixItemMap::GenerateToken(const std::string& name) {
  uint64_t token = (((uint64_t)(ThreadId::Get()) << 32) | (++token_id_));
  MatrixBucketItemMap& bucket = GetBucket(token);
  if (0 != bucket.GenerateToken(token, name)) {
    LOG_ERROR(logger, "MatrixItemMap::GenerateToken fail" 
        << ", token:" << token << ", name:" << name);
    return 0;
  }
  return token;
}

int MatrixItemMap::FetchToken(uint64_t token, MatrixItem** item) {
  MatrixBucketItemMap& bucket = GetBucket(token);
  return bucket.FetchToken(token, item);
}

} //namespace matrix
} //namespace ade
