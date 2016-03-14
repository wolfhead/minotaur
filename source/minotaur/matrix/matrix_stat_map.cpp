/**
 * @file matrix_stat_map.cpp
 * @author Wolfhead
 */
#include "matrix_stat_map.h"
#include <sstream>
#include "../common/time_util.h"

namespace ade { namespace matrix {
  
MatrixStatMap::MatrixStatMap(uint32_t start_time_sec) 
    : start_time_sec_(start_time_sec) 
    , freeze_(false) 
    , freeze_time_(0) {
}

void MatrixStatMap::Freeze() {
  freeze_ = true;
  freeze_time_ = time(NULL);
  std::ostringstream oss;
  Dump(oss);
  dump_string_ = oss.str();

  std::ostringstream oss_simple;
  DumpSimple(oss_simple);
  dump_string_simple_ = oss_simple.str();
}

void MatrixStatMap::Dump(std::ostream& os) const {
  os << "{";
  os << "\n  \"__version__\": " << freeze_time_
     << ",\n  \"__version_time__\": \"" << Time::Format<256>("%Y-%m-%d %H:%M:%S", freeze_time_) << "\"";
  for (const auto& name : stat_name_set_) {
    auto it = stat_map_.find(name);
    if (it == stat_map_.end()) {
      continue;
    }
    os << ",\n  \"" << name << "\": " << it->second;
  }
  os << "}\n";
}

void MatrixStatMap::DumpSimple(std::ostream& os) const {
  for (const auto& name : stat_name_set_) {
    auto it = stat_map_.find(name);
    if (it == stat_map_.end()) {
      continue;
    }
    it->second.DumpSimple(name, os);
  }
}

const std::string& MatrixStatMap::ToString() const {
  const static std::string k_not_ready = "\"Not Ready\"";
  if (!freeze_) {
    return k_not_ready;
  } else {
    return dump_string_;
  }
}

const std::string& MatrixStatMap::ToStringSimple() const {
   const static std::string k_not_ready = "\"Not Ready\"";
  if (!freeze_) {
    return k_not_ready;
  } else {
    return dump_string_simple_;
  }  
}

void MatrixStatMap::Inherit(const MatrixStatMap& map) {
  for (const auto& stat : map.stat_map_) {
    if (stat.second.IsPersistent()) {
      stat_name_set_.insert(stat.first);
      stat_map_.insert(std::make_pair(stat.first, stat.second));
    }
  }

}

std::ostream& operator << (std::ostream& os, const MatrixStatMap& stat_map) {
  stat_map.Dump(os);
  return os;
}

} //namespace matrix
} //namespace ade
