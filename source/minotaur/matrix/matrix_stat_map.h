#ifndef _MINOTAUR_MATRIX_STAT_MAP_H_
#define _MINOTAUR_MATRIX_STAT_MAP_H_
/**
 * @file matrix_stat_map.h
 * @author Wolfhead
 */
#include <set>
#include <unordered_map>
#include "matrix_stat.h"

namespace ade { namespace matrix {

class MatrixStatMap {
 public:
  typedef std::unordered_map<std::string, MatrixStat> StatMap;
  typedef std::set<std::string> StatNameSet;

  MatrixStatMap(uint32_t start_time_sec);

  inline uint32_t GetStartTime() const {return start_time_sec_;}

  inline void Set(const std::string& name, uint32_t value, bool persistent) {
    GetMatrixStat(name, persistent).Set(value);
  }

  inline void Add(const std::string& name, uint32_t value, bool persistent) {
    GetMatrixStat(name, persistent).Add(value);
  }

  inline void Sub(const std::string& name, uint32_t value, bool persistent) {
    GetMatrixStat(name, persistent).Sub(value);
  }

  inline void Reset(const std::string& name) {
    GetMatrixStat(name, false).Reset();
  }

  inline void TimeDistrubute(const std::string& name, int64_t value, bool persistent) {
    GetMatrixStat(name, persistent).TimeDistribute(value);
  }

  inline void TimeDistrubute(
      const std::string& name, const std::string& result, 
      int64_t value, bool persistent) {
    GetMatrixStat(name, persistent).TimeDistribute(result, value);
  }

  MatrixStat& GetMatrixStat(const std::string& name, bool persistent) {
    auto it = stat_map_.find(name);
    if (it == stat_map_.end()) {
      stat_name_set_.insert(name);
      return stat_map_.insert(std::make_pair(name, MatrixStat(start_time_sec_, persistent))).first->second;
    }
    return it->second;
  }

  void Freeze();

  void Dump(std::ostream& os) const;
  void DumpSimple(std::ostream& os) const;

  const std::string& ToString() const;
  const std::string& ToStringSimple() const;

  void Inherit(const MatrixStatMap& map);

 private:
  uint32_t start_time_sec_;
  StatMap stat_map_;
  StatNameSet stat_name_set_;

  bool freeze_;
  uint32_t freeze_time_;
  std::string dump_string_;
  std::string dump_string_simple_;
};

std::ostream& operator << (std::ostream& os, const MatrixStatMap& stat_map); 

} //namespace matrix
} //namespace ade

#endif // _MINOTAUR_MATRIX_STAT_MAP_H_q
