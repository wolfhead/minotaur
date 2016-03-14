/**
 * @file matrix_stat.cpp
 * @author Wolfhead
 */
#include "matrix_stat.h"
#include <iomanip>

namespace ade { namespace matrix {

MatrixStat::MatrixStat(uint32_t start_time_sec, bool persistent) 
    : persistent_(persistent)
    , start_time_sec_(start_time_sec)
    , count_(0)
    , value_(0) 
    , max_(0)
    , min_(0xFFFFFFFF) {
}

MatrixStat::~MatrixStat() {
}

void MatrixStat::TimeDistribute(const std::string& result, int64_t value) {
  TimeDistribute(value);
  if (result.empty()) {
    return;
  }

  if (child_.find(result) == child_.end()) {
    child_[result] = MatrixStat(start_time_sec_, persistent_);
  }
  child_[result].TimeDistribute(value);
}

void MatrixStat::TimeDistribute(int64_t value) {
  if (distribution_.size() != kTotalBucket) {// 10 seconds, each bucket 100 us
    distribution_.resize(kTotalBucket);
  }

  MaxMin(value);
  ++count_;
  value_ += value;
  uint32_t bucket = (uint32_t)(value / kBucketUsec);
  if (bucket >= kTotalBucket) {
    bucket = kTotalBucket - 1; 
  }
  ++distribution_[bucket];
}

uint64_t MatrixStat::GetTimeDistribute(double percent) const {
  uint32_t count = (uint32_t)(percent * count_);
  uint32_t sum_count = 0;
  uint32_t bucket = 0;
  for (; bucket != distribution_.size() && sum_count < count; ++bucket) {
    sum_count += distribution_[bucket];
  }
  bucket = bucket ? bucket - 1 : 0;

  return bucket * kBucketUsec;
}

void MatrixStat::Dump(std::ostream& os) const {
  os.setf(std::ios::fixed);
  os << std::setprecision(2); 
  os << "{\"qps\": " << GetQps()
     << ", \"count\": " << GetCount()
     << ", \"avg\": " << GetAvg()
     << ", \"max\": " << GetMax()
     << ", \"min\": " << GetMin();
  if (HasTimeDistribute()) {
    os << ", \"99\": " << GetTimeDistribute(0.99)
       << ", \"95\": " << GetTimeDistribute(0.95)
       << ", \"90\": " << GetTimeDistribute(0.9)
       << ", \"80\": " << GetTimeDistribute(0.8)
       << ", \"50\": " << GetTimeDistribute(0.5);
  } else {
    os << ", \"value\": " << GetValue();
  }
  if (HasResult()) {
    os << ",\n    \"Result\": {";
    bool first = true;
    for (const auto& pair : child_) {
      if (first == true) {
        first = false;
        os << "\n      ";
      } else {
        os << ",\n      ";
      }
      os << "\"" << pair.first << "\": ";
      pair.second.Dump(os);
    }
    os << "}";
  }
  os << "}";
}

void MatrixStat::DumpSimple(const std::string& parent, std::ostream& os) const {
  os.setf(std::ios::fixed);
  os << std::setprecision(2); 
  os << "[matrix_stat] {\"name\": \"" << parent << "\""
     << ", \"qps\": " << GetQps()
     << ", \"count\": " << GetCount()
     << ", \"avg\": " << GetAvg()
     << ", \"max\": " << GetMax()
     << ", \"min\": " << GetMin();
  if (HasTimeDistribute()) {
    os << ", \"99\": " << GetTimeDistribute(0.99)
       << ", \"95\": " << GetTimeDistribute(0.95)
       << ", \"90\": " << GetTimeDistribute(0.9)
       << ", \"80\": " << GetTimeDistribute(0.8)
       << ", \"50\": " << GetTimeDistribute(0.5);
  } else {
    os << ", \"value\": " << GetValue();
  }
  os << "}" << std::endl;

  if (HasResult()) {
    for (const auto& pair : child_) {
      std::string sub = parent + "." + pair.first; 
      pair.second.DumpSimple(sub, os);
    }
  }
}

std::ostream& operator << (std::ostream& os, const MatrixStat& stat) {
  stat.Dump(os);
  return os;
}

} //namespace matrix
} //namespace ade
