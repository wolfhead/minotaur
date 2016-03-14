#ifndef _MINOTAUR_MATRIX_SCOPE_H_
#define _MINOTAUR_MATRIX_SCOPE_H_
/**
 * @file matrix_scope.h
 * @author Wolfhead
 */
#include "matrix.h"
#include "matrix_item.h"
#include "../common/time_util.h"

namespace ade { namespace matrix {

class MatrixScope {
 public:
  enum {
    kModeLazy, /* matrix scope will not set the result of matrix */
    kModeAutoSuccess, /* if no result is set, the matrix will have result of "OK" */
    kModeAutoFail, /* if no result is set, the matrix will have result of "ERR" */
  };

  MatrixScope(const std::string& name, int mode = kModeLazy)
      : item_(new MatrixItem(Matrix::kTimeDistribute, name, Time::GetMicrosecond()))
      , mode_(mode) { 
  }


  ~MatrixScope() {
    static const std::string success = "OK";
    static const std::string fail = "ERR";

    item_->val = Time::GetMicrosecond() - item_->val;
    if (item_->result.empty() && mode_ != kModeLazy) {
      if (mode_ == kModeAutoFail) item_->result = fail;
      if (mode_ == kModeAutoSuccess) item_->result = success;
    }

    if (GlobalMatrix::Ready()) {
      GlobalMatrix::Instance().SendToCollector(item_);
    }
  }

  inline void SetOkay(bool result) {
    static const std::string success = "OK";
    static const std::string fail = "ERR";
    item_->result = result ? success : fail;
  }

  inline void SetResult(const std::string& result) {
    item_->result = result;
  }

 private:
  MatrixScope(const MatrixScope& scope);
  MatrixScope& operator = (const MatrixScope& scope);

  MatrixItem* item_;
  int mode_;
};

class MatrixCounter {
 public:
  inline static void Set(const std::string& name, uint32_t value) {
    if (GlobalMatrix::Ready()) GlobalMatrix::Instance().Set(name, value);
  }

  inline static void Add(const std::string& name, uint32_t value) {
    if (GlobalMatrix::Ready()) GlobalMatrix::Instance().Add(name, value);
  }

  inline static void Sub(const std::string& name, uint32_t value) {
    if (GlobalMatrix::Ready()) GlobalMatrix::Instance().Sub(name, value);
  }

  inline static void Reset(const std::string& name) {
    if (GlobalMatrix::Ready()) GlobalMatrix::Instance().Reset(name);
  }

  inline static void Incr(const std::string& name) {
    if (GlobalMatrix::Ready()) GlobalMatrix::Instance().Add(name, 1);
  }

  inline static void Decr(const std::string& name) {
    if (GlobalMatrix::Ready()) GlobalMatrix::Instance().Sub(name, 1);
  }

  inline static void Distribution(const std::string& name, uint32_t value) {
    if (GlobalMatrix::Ready()) GlobalMatrix::Instance().SendToCollector(new MatrixItem(Matrix::kTimeDistribute, name, value));
  }
};

} //namespace matrix
} //namespace ade

#endif //_MINOTAUR_MATRIX_SCOPE_H_
