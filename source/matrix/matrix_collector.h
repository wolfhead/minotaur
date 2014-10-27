#ifndef _MINOTAUR_MATRIX_LOCAL_MATRIX_H_
#define _MINOTAUR_MATRIX_LOCAL_MATRIX_H_

#include <stdint.h>
#include <vector>
#include <boost/thread.hpp>
#include "matrix_item_map.h"
#include "matrix_stat_map.h"

namespace minotaur { namespace matrix {

class MatrixCollector {
 public:
  typedef std::vector<MatrixCollector*> ChildVec;
  typedef uint64_t MaterixItemId;

  MatrixCollector();
  ~MatrixCollector();

  MatrixCollector* GetParent() const {return parent_;}
  void RegisterParent(MatrixCollector* matrix_collector);
  void UnregisterParent();

  const ChildVec& GetChildVec() const {return child_vec_;} 
  void RegisterChild(MatrixCollector* matrix_collector);
  void UnregisterChild(MatrixCollector* matrix_collector);

  void SetLocal(const std::string& name, int32_t count);
  void AddLocal(const std::string& name, int32_t count);
  void SubLocal(const std::string& name, int32_t count);
  void ResetLocal(const std::string& name);

  void ModifyShared(const std::string& name, int32_t count);
  void ResetShared(const std::string& name);

  MaterixItemId BeginLocal(const std::string& name);
  void EndLocal(MaterixItemId id, const std::string& result);
  void EndLocal(MaterixItemId id);

  MaterixItemId BeginShared(const std::string& name);
  void EndShared(MaterixItemId id, const std::string& result);
  void EndShared(MaterixItemId id);

 private:
  MatrixCollector* parent_;

  ChildVec child_vec_;
  boost::mutex child_lock_;

  MatrixItemMap item_map_;
  MatrixStatMap stat_map_;
};

} //namespace matrix
} //namesapce minotaur

#endif // _MINOTAUR_MATRIX_LOCAL_MATRIX_H_
