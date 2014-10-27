/**
 * @file matrix_item_map.cpp
 * @author Wolfhead
 */
#include "matrix_item_map.h"
#include "../common/time_util.h"

namespace minotaur { namespace matrix {

MatrixItemMap::MatrixItemMap() {
  maps_ = new ItemMap[bucket_count];
  locks_ = new LockType[bucket_count];
}

MatrixItemMap::~MatrixItemMap() {
  delete [] maps_;
  delete [] locks_;
}

uint64_t MatrixItemMap::Create(const std::string& name) {
  uint16_t bucket_id = GetBucket();
  uint32_t local_id = GetLocalId(); 
  locks_[bucket_id].lock();

  maps_[bucket_id].insert(std::make_pair(local_id, MatrixItem(name)));

  locks_[bucket_id].unlock();
  return BuildMaxtrixItemId(bucket_id, local_id);
}

bool MatrixItemMap::Get(uint64_t id, MatrixItem* item) {
  uint16_t bucket_id = GetBucketIdFromMaxtrixItemId(id);
  uint32_t local_id = GetLocalIdFromMaxtrixItemId(id);
  auto& lock = locks_[bucket_id];
  auto& map = maps_[bucket_id];

  lock.lock();
  auto pos = map.find(local_id);
  if (pos == map.end()) {
    lock.unlock();
    return false;
  }

  *item = pos->second;
  map.erase(pos);
  lock.unlock();

  return true;
}

void MatrixItemMap::ClearExpire(uint32_t usec_timeout) {
  Time::Microsecond current_usec = Time::GetMicrosecond();
  for (uint16_t bucket_id = 0; bucket_id != bucket_count; ++bucket_id) {
    ClearOneBucket(bucket_id, current_usec, usec_timeout); 
  }
}

void MatrixItemMap::ClearOneBucket(
    uint16_t bucket_id,
    uint64_t current_usec, 
    uint32_t usec_timeout) {
  auto& lock = locks_[bucket_id];
  auto& map = maps_[bucket_id];

  lock.lock();
  for (ItemMap::iterator it = map.begin(); it != map.end(); ++it) {
    if (current_usec - it->second.time_start_usec > usec_timeout) {
      it = map.erase(it);
    }
  }
  lock.unlock();
}


} //namespace matrix
} //namespace minotaur
