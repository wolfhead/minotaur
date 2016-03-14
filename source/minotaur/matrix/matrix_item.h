#ifndef _MINOTAUR_MATRIX_ITEM_H_
#define _MINOTAUR_MATRIX_ITEM_H_
/**
 * @file matrix_item.h
 * @author Wolfhead
 */
#include <stdint.h>
#include <iostream>
#include <string>

namespace ade { namespace matrix {

class MatrixItem {
 public:
  MatrixItem() = default;

  MatrixItem(int8_t _operation, const std::string& _name, int64_t _val, bool _persistent = false)
      : persistent(_persistent)
      , operation(_operation)
      , val(_val)
      , name(_name) {
  }

  bool persistent;
  int8_t operation;
  int64_t val;
  std::string name;
  std::string result;

  void Dump(std::ostream& os) const;
};

std::ostream& operator << (std::ostream& os, const MatrixItem& item);

} //namespace matrix
} //namespace ade

#endif// _MINOTAUR_MATRIX_ITEM_H_
