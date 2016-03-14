#ifndef _MINOTAUR_MATRIX_MATRIX_H_
#define _MINOTAUR_MATRIX_MATRIX_H_
/**
 * @file matrix.h
 * @author Wolfhead
 */
#include <stdint.h>
#include <string>
#include <boost/shared_ptr.hpp>
#include "../common/logger.h"
#include "../common/xml_macro.h"

namespace ade { namespace matrix {

class Matrix;
class MatrixItem;
class MatrixItemMap;
class MatrixCollector;
class MatrixStatMap;

class GlobalMatrix {
 public:
  inline static Matrix& Instance() {
    return *global_matrix_;
  }

  inline static bool Ready() {
    return global_matrix_ != NULL;
  }

  static int Init(
      uint32_t item_map_bucket,
      uint32_t queue_bucket,
      uint32_t queue_size);

  static int InitFromXml(tinyxml2::XMLElement* xml);

  static int Destroy();

 private:
  LOGGER_CLASS_DECL(logger);

  static Matrix* global_matrix_;
};

class Matrix {
 public:

  typedef boost::shared_ptr<MatrixStatMap> MatrixStatMapPtr;

  enum {
    kSet = 0,
    kAdd,
    kSub,
    kReset,
    kTimeDistribute,
  };

  Matrix(
      MatrixItemMap* item_map,
      MatrixCollector* collector);

  ~Matrix();

  void Set(const std::string& name, uint32_t value);

  void Add(const std::string& name, uint32_t value);

  void Sub(const std::string& name, uint32_t value);

  void PersistentSet(const std::string& name, uint32_t value);

  void PersistentAdd(const std::string& name, uint32_t value);

  void PersistentSub(const std::string& name, uint32_t value);

  void Reset(const std::string& name);

  uint64_t MarkBegin(const std::string& name);

  int MarkEnd(uint64_t token);

  int MarkEnd(uint64_t token, const std::string& result);

  int MarkEnd(uint64_t token, int64_t result);

  int SendToCollector(const MatrixItem* item);

  MatrixStatMapPtr GetMatrixStatMap();

 private:
  LOGGER_CLASS_DECL(logger);

  MatrixItemMap* item_map_;
  MatrixCollector* collector_;
};

} //namespace matrix
} //namespace ade

#endif //_MINOTAUR_MATRIX_MATRIX_H_
