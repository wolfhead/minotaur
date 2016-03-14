#ifndef _MINOTAUR_MATRIX_STAT_H_
#define _MINOTAUR_MATRIX_STAT_H_
/**
 * @file matrix_stat.h
 * @author Wolfhead
 */
#include <string>
#include <vector>
#include <set>
#include <map>
#include <iostream>

namespace ade { namespace matrix {

class MatrixStat {
 public:
  enum {
    kTotalUsec = 10 * 1000 * 1000,
    kBucketUsec = 100,
    kTotalBucket = kTotalUsec / kBucketUsec,
  };

  MatrixStat()
      : start_time_sec_(0)
      , count_(0)
      , value_(0)
      , max_(0)
      , min_(0xFFFFFFFF) {
  }

  MatrixStat(const MatrixStat&) = default;
  MatrixStat(uint32_t start_time_sec, bool persistent);
  ~MatrixStat();

  inline void Set(int64_t value) {
    ++count_; 
    value_ = value;
    MaxMin(value_);
  }

  void Add(int64_t value) {
    ++count_; 
    value_ += value;
    MaxMin(value_);
  }

  void Sub(int64_t value) {
    ++count_; 
    value_ -= value;
    MaxMin(value_);
  }

  void Reset() {
    value_ = 0;
    count_ = 0;
    max_ = 0;
    min_ = 0xFFFFFFFF;
  }

  inline bool IsPersistent() const {return persistent_;}

  void TimeDistribute(const std::string& result, int64_t value);
  void TimeDistribute(int64_t value);

  void Dump(std::ostream& os) const;
  void DumpSimple(const std::string& name, std::ostream& os) const;

  inline bool HasTimeDistribute() const {return distribution_.size() != 0;}
  inline bool HasResult() const {return child_.size() != 0;}
  inline double GetQps() const {
    int32_t elapse = time(NULL) - start_time_sec_;
    elapse = elapse <= 0 ? 0 : elapse; 
    return elapse ? (double)count_ / elapse : (double)0;
  }

  inline double GetAvg() const {return count_ ? (double)value_ / count_ : (double)0;}
  inline uint32_t GetCount() const {return count_;}
  inline int64_t GetValue() const {return value_;}
  inline int64_t GetMin() const {return min_;}
  inline int64_t GetMax() const {return max_;}
  uint64_t GetTimeDistribute(double percent) const;

 private:
  void MaxMin(int64_t value) {
    max_ = std::max(value, max_);
    min_ = std::min(value, min_);
  }

  bool persistent_;
  uint32_t start_time_sec_;
  uint32_t count_;
  int64_t value_;
  int64_t max_;
  int64_t min_;
  std::vector<uint32_t> distribution_;
  std::map<std::string, MatrixStat> child_;
};

std::ostream& operator << (std::ostream& os, const MatrixStat& stat);

} //namespace matrix
} //namespace ade

#endif //_MINOTAUR_MATRIX_STAT_H_
