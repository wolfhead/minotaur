/**
 * @file slice.cpp
 * @author Wolfhead
 */
#include "slice.h"

namespace ade {

std::ostream& operator << (std::ostream& os, const Slice& s) {
  os << s.str();
  return os;
}

} //namespace ade
