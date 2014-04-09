#ifndef MINOTAUR_LOCKFREE_TAGGED_PTR_HPP
#define MINOTAUR_LOCKFREE_TAGGED_PTR_HPP
/**
  @file tagged_ptr.hpp
  @author Wolfhead
*/

#include <stdint.h>

namespace minotaur {
namespace lockfree {

/**
  @notice this tagged_ptr will only work on x64 env
*/
template <class T>
class tagged_ptr {
 public:
  typedef uint64_t compressed_ptr_t;
  typedef uint16_t tag_t;

  static const compressed_ptr_t ptr_mask = 0xffffffffffffUL; //(1L<<48L)-1;
  static const int tag_index = 3;

  tagged_ptr() : ptr_(0) {}

  tagged_ptr(const tagged_ptr& ptr) = default;

  tagged_ptr(const T* p, tag_t t) : ptr_(pack_ptr(p, t)) {}

  void set(const T* p, tag_t t) {ptr_ = pack_ptr(p, t);}

  T* get_ptr() const {
    return extract_ptr(ptr_);
  }

  void set_ptr(const T* p) {
    tag_t tag = get_tag();
    ptr_ = pack_ptr(p, tag);
  }

  tag_t get_tag() const {
    return extract_tag(ptr_);
  }

  void set_tag(tag_t t) {
    T* p = get_ptr();
    ptr_ = pack_ptr(p, t);
  }
  
  bool operator == (const volatile tagged_ptr& p) const {
    return ptr_ == p.ptr_;
  }

  bool operator != (const volatile tagged_ptr& p) const {
    return ptr_ != p.ptr_;
  }

  T& operator*() {
    return *get_ptr();
  }

  T* operator->() {
    return get_ptr();
  }

  operator bool() {
    return get_ptr() != 0;
  }

  tagged_ptr next_tag() const {
    return tagged_ptr(get_ptr(), get_tag() + 1);
  }

 private:

  union cast_unit
  {
    compressed_ptr_t value;
    tag_t tag[4];
  };

  static T* extract_ptr(const volatile compressed_ptr_t& i)
  {
    return (T*)(i & ptr_mask);
  }

  static tag_t extract_tag(const volatile compressed_ptr_t& i)
  {
    cast_unit cu;
    cu.value = i;
    return cu.tag[tag_index];
  }

  static compressed_ptr_t pack_ptr(const T * ptr, int tag)
  {
    cast_unit ret;
    ret.value = compressed_ptr_t(ptr);
    ret.tag[tag_index] = tag;
    return ret.value;
  }

  compressed_ptr_t ptr_;
};

} //namespace lockfree
} //namespace minotaur

#endif // MINOTAUR_LOCKFREE_TAGGED_PTR_HPP
