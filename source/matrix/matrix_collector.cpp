/**
 * @file matrix_collector.cpp
 * @author Wolfhead
 */
#include "matrix_collector.h"

namespace minotaur { namespace matrix {

MatrixCollector::MatrixCollector() 
    : parent_(NULL) {
}

MatrixCollector::~MatrixCollector() {
}

void MatrixCollector::RegisterParent(MatrixCollector* matrix_collector) {
  boost::unique_lock<boost::mutex> lock(child_lock_);

  if (std::find(child_vec_.begin(), child_vec_.end(), matrix_collector) != child_vec_.end()) {
    return;
  }

  child_vec_.push_back(matrix_collector);
  return;
}

void MatrixCollector::UnregisterParent(MatrixCollector* matrix_collector) {
  boost::unique_lock<boost::mutex> lock(child_lock_);
  auto pos = std::find(child_vec_.begin(), child_vec_.end(), matrix_collector);

  if (pos == child_vec_.end()) {
    return;
  }

  child_vec_.erase(pos);
  return;
}

void MatrixCollector::RegisterParent(MatrixCollector* matrix_collector) {
  parent_ = matrix_collector;
  parent_->RegisterChild(this);
}

void MatrixCollector::UnregisterParent() {
  if (!parent_) {
    return;
  }
  parent_->UnregisterChild(this); 
  parent_ = NULL;
}

void MatrixCollector::SetLocal(const std::string& name, int64_t count) {
  stat_map_.Set(name, count); 
}

void MatrixCollector::AddLocal(const std::string& name, int64_t count) {
  stat_map_.Add(name, count); 
}

void MatrixCollector::SubLocal(const std::string& name, int64_t count) {
  stat_map_.Sub(name, count);
}

void MatrixCollector::ResetLocal(const std::string& name) {
  stat_map_.Reset(name);
}


MaterixItemId MatrixCollector::BeginLocal(const std::string& name) {
}


void EndLocal(MaterixItemId id, const std::string& result);
void EndLocal(MaterixItemId id);







} //namespace matrix
} //namespace minotaur
