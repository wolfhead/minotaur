/**
 * @file matrix_item.cpp
 * @author Wolfhead
 */
#include "matrix_item.h"

namespace ade { namespace matrix {

void MatrixItem::Dump(std::ostream& os) const {
  os << "{\"operation\": " << (int)operation
     << ",\"persistenc\": " << persistent
     << ",\"val\": " << val
     << ",\"name\": \"" << name << "\""
     << ",\"result\": \"" << result << "\""
     << "}";
}

std::ostream& operator << (std::ostream& os, const MatrixItem& item) {
  item.Dump(os);
  return os;
}

} //namespace matrix
} //namespace ade
