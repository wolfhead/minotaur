#ifndef _MINOTAUR_MATRIX_STAT_MAP_H_
#define _MINOTAUR_MATRIX_STAT_MAP_H_
/**
 * @file matrix_stat_map.h
 * @author Wolfhead
 */
#include <stdint.h>
#include <string>
#include <boost/unordered_map.hpp>

namespace minotaur { namespace matrix {

class MatrixStat {
  uint32_t pending;
  uint32_t finish;
};

class MatrixStatMap {
 public:
  typedef boost::unordered_map<uint32_t, MatrixStat> ItemMap;

  void Set(const std::string& name, uint32_t count);

  void Add(const std::string& name, uint32_t count);

  void Sub(const std::string& name, uint32_t count);

  void Reset(const std::string& name);

  void Modify(
      const std::string& name,
      uint32_t count,
      uint32_t latency_usec); 


};

} //namespace matrix
} //namespace minotaur

#endif // _MINOTAUR_MATRIX_STAT_MAP_H_q
