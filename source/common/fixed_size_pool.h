#ifndef _MINOTAUR_COMMON_FIXED_SIZZE_POOL_
#define _MINOTAUR_COMMON_FIXED_SIZZE_POOL_
/**
 * @file fixed_size_pool.h
 * @author Wolfhead
 */
#include "../lockfree/freelist.hpp"

namespace minotaur {

template<typename T>
class FixedSizePool {
 public:
  FixedSizePool(uint32_t count) 
      : freelist_(count){
  }

  T* Alloc(uint64_t* key) {
    *key = freelist_.alloc_key();
    return freelist_.get_key(*key);
  }

  template<typename A1>
  T* Alloc(const A1& arg1, uint64_t* key) {
    *key = freelist_.alloc_key(arg1);
    return freelist_.get_key(*key);
  }

  template<typename A1, typename A2>
  T* Alloc(const A1& arg1, const A2& arg2, uint64_t* key) {
    *key = freelist_.alloc_key(arg1, arg2);
    return freelist_.get_key(*key);
  }

  template<typename A1, typename A2, typename A3>
  T* Alloc(const A1& arg1, const A2& arg2, const A3& arg3, uint64_t* key) {
    *key = freelist_.alloc_key(arg1, arg2, arg3);
    return freelist_.get_key(*key);
  }

  T* GetKey(uint64_t key) {
    return freelist_.get_key(key);
  }

  bool DestroyKey(uint64_t key) {
    return freelist_.dealloc_key(key);
  }

  bool Destroy(T* p) {
    return freelist_.destory(p);
  }

 private:
  lockfree::fix_sized_freelist<T> freelist_; 
};

};

#endif //_MINOTAUR_COMMON_FIXED_SIZZE_POOL_
