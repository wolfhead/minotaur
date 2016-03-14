#ifndef MINOTAUR_COMMON_SLICE_H
#define MINOTAUR_COMMON_SLICE_H
/**
 * @file slice.h
 * @author Wolfhead
 */
#include <stdint.h>
#include <string.h>
#include <string>

namespace ade {

class Slice {
 public:
  Slice() : data_(""), size_(0) {}
  Slice(const Slice& s) : data_(s.data_), size_(s.size_) {}
  Slice(const void* d, uint32_t n) : data_((const char *)d), size_(n) {}
  Slice(const char* s) : data_(s), size_(strlen(s)) {}
  explicit Slice(const std::string& s) : data_(s.data()), size_(s.size()) {}

  Slice& operator= (const Slice& s) {
    if (&s == this) return *this;
  
    data_ = s.data_;
    size_ = s.size_;
    return *this;
  }

  Slice& operator= (const std::string& s) {
    data_ = s.data();
    size_ = s.size();
    return *this;
  }

  Slice& operator= (const char* s) {
    data_ = s;
    size_ = strlen(s);
    return *this;
  }

  inline void set(const void* d, size_t n) {data_ = (const char *)d; size_ = n;}

  inline void set(const std::string& s) {data_ = s.data(); size_ = s.size();}

  inline const char* data() const { return data_; }

  inline size_t size() const { return size_; }

  inline bool empty() const { return size_ == 0; }

  inline void clear() { data_ = ""; size_ = 0; }

  inline std::string str() const { return std::string(data_, size_);}

  // Three-way comparison.  Returns value:
  // <  0 iff < s,
  // == 0 iff == s,
  // >  0 iff > s
  inline int compare(const Slice& s) const {
    const size_t min_len = (size_ < s.size_) ? size_ : s.size_;
    int r = memcmp(data_, s.data_, min_len);
    if (r == 0) {
      if (size_ < s.size_) r = -1;
      else if (size_ > s.size_) r = +1;
    }
    return r;
  }

  inline bool equal(const Slice& s) const {
    if (size_ != s.size()) return false;
    return memcmp(data_, s.data_, size_) == 0;
  }

  bool operator == (const Slice& s) const {
    return equal(s);
  }

  bool operator != (const Slice& s) const {
    return !equal(s);
  }

  bool operator > (const Slice& s) const {
    return compare(s) > 0;
  }

  bool operator < (const Slice& s) const {
    return compare(s) < 0;
  }

  bool operator >= (const Slice& s) const {
    return compare(s) >= 0;
  }

  bool operator <= (const Slice& s) const {
    return compare(s) <= 0;
  }

  void trim(uint32_t size) {
    data_ += std::min(size, size_);
    size_ -= std::min(size, size_);
  }

  Slice trim_copy(uint32_t size) {
    return Slice(data_ + std::min(size, size_), size_ - std::min(size, size_));
  }

  Slice strip(const char* target) {
    const char* p = data_;
    while (p != data_ + size_) {
      if (strchr(target, *p) == NULL) {
        break;
      }
      ++p;
    }
    return Slice(p, size_ - (p - data_));
  }

 private:
  const char* data_;
  uint32_t size_;
};

std::ostream& operator << (std::ostream& os, const Slice& s);

} //namespace ade

#endif //MINOTAUR_COMMON_SLICE_H
