#ifndef _MINOTAUR_LOCKFREE_FREELIST_HPP_
#define _MINOTAUR_LOCKFREE_FREELIST_HPP_
/**
  @file freelist.hpp
  @author Wolfhead
*/
#include <assert.h>
#include <algorithm>
#include <atomic>
#include <mutex>
#include <type_traits>
#include "tagged_ptr.hpp"

namespace minotaur {
namespace lockfree {
namespace __detail__ {

template<bool pod>
struct alloc_triats {};

template<>
struct alloc_triats<true> {
  template<typename T>
  static void construct(T*) {}

  template<typename T, typename A1>
  static void construct(T* buffer, const A1& arg1) {
    new (buffer) T(arg1);
  }

  template<typename T>
  static void destruct(T*) {}
};

template<>
struct alloc_triats<false> {
  template<typename T>
  static void construct(T* buffer) {
    new (buffer) T;
  }

  template<typename T, typename A1>
  static void construct(T* buffer, const A1& arg1) {
    new (buffer) T(arg1);
  }

  template<typename T, typename A1, typename A2>
  static void construct(T* buffer, const A1& arg1, const A2& arg2) {
    new (buffer) T(arg1, arg2);
  }

  template<typename T, typename A1, typename A2, typename A3>
  static void construct(T* buffer, const A1& arg1, const A2& arg2, const A3& arg3) {
    new (buffer) T(arg1, arg2, arg3);
  }

  template<typename T>
  static void destruct(T* buffer) {
    buffer->~T();
  }
};

template<typename T>
struct construct_traits {
  static void construct(T* buffer) {
    alloc_triats<std::is_pod<T>::value>::construct(buffer);
  }

  template<typename A1>
  static void construct(T* buffer, const A1& arg1) {
    alloc_triats<std::is_pod<T>::value>::construct(buffer, arg1);
  }

  template<typename A1, typename A2>
  static void construct(T* buffer, const A1& arg1, const A2& arg2) {
    alloc_triats<std::is_pod<T>::value>::construct(buffer, arg1, arg2);
  }

  template<typename A1, typename A2, typename A3>
  static void construct(T* buffer, const A1& arg1, const A2& arg2, const A3& arg3) {
    alloc_triats<std::is_pod<T>::value>::construct(buffer, arg1, arg2, arg3);
  }

  static void destruct(T* buffer) {
    alloc_triats<std::is_pod<T>::value>::destruct(buffer);
  }
};

} //namespace __detail__


class block_allocator {
 public:
  struct tagged_node;
  typedef tagged_ptr<tagged_node> tagged_node_ptr;

  // when the tagged node is in the freelist
  // member next point to the next free node
  // when the tagged node is allocated
  // member next hold the value of current tagged pointer
  struct tagged_node {
    uint64_t next;
    uint8_t data[0];
  };

  block_allocator(uint32_t batch_count, uint32_t block_size) 
      : batch_count_(batch_count)
      , block_size_(get_block_size(block_size)) {
    add_mem_buffer();    
  }

  ~block_allocator() {
    for (uint8_t* p : pool_) {
      destruct_mem_buffer(p);
    }
  }

  uint32_t block_size() const {return block_size_;}

  uint8_t* alloc() {
    uint64_t key = alloc_key();
    return get_key(key);
  }

  bool dealloc(uint8_t* p) {
    tagged_node* node = (tagged_node*)((uint8_t*)p - sizeof(uint64_t));
    return dealloc_key(node->next);
  }

  uint64_t alloc_key() {
    tagged_node_ptr node = alloc_node();
    if (!node) return 0;
    return node.get_raw();
  }

  bool dealloc_key(uint64_t key) {
    tagged_node_ptr node(key);
    if (!check_node(node)) return false;
    dealloc_node(node);
    return true;
  }

  static uint8_t* get_key(uint64_t key) {
    if (key == 0) return NULL;
    tagged_node_ptr node(key);
    if (!check_node(node)) return NULL;
    return node->data;
  } 

 private:
  static uint32_t get_block_size(uint32_t block_size) {
    return sizeof(tagged_node) + block_size;
  }

  static uint8_t* alloc_mem_buffer(uint32_t item_count, uint32_t item_size) {
    return new uint8_t[item_size * item_count];
  }

  void add_mem_buffer() {
    uint8_t* buffer = alloc_mem_buffer(batch_count_, block_size_);
    init_free_list_on_mem_buffer(buffer);

    std::lock_guard<std::mutex> gurad(pool_lock_);
    pool_.push_back(buffer);    
  }

  static void destruct_mem_buffer(uint8_t* p) {
    delete [] p;
  }

  void init_free_list_on_mem_buffer(uint8_t* buffer) {
    for (uint32_t i = 0; i != batch_count_; ++i) {
      tagged_node* node = (tagged_node*)(buffer + i * block_size_);
      tagged_node_ptr p(node, 0);
      dealloc_node(p);
    }
  }

  tagged_node_ptr alloc_node() {
    tagged_node_ptr old_head;

    do {
      old_head = head_.load(std::memory_order_acquire);
      if (!old_head) {
        // add another buffer
        add_mem_buffer();
        old_head = head_.load(std::memory_order_acquire);
        continue; 
      } 
    } while (!head_.compare_exchange_weak(old_head, tagged_node_ptr(old_head->next)));
        
    old_head->next = old_head.get_raw();
    return old_head;
  }

  void dealloc_node(tagged_node_ptr node) {
    tagged_node_ptr old_head;
    tagged_node_ptr new_head(node.next_tag());

    do {
      old_head = head_.load(std::memory_order_acquire);
      new_head->next = old_head.get_raw();

    } while (!head_.compare_exchange_weak(old_head, new_head));
  }

  static bool check_node(tagged_node_ptr node) {
    return node->next == node.get_raw();
  }

  uint32_t batch_count_;
  uint32_t block_size_;
  std::vector<uint8_t*> pool_;
  std::mutex pool_lock_;
  std::atomic<tagged_node_ptr> head_;
};


template<class T>
class freelist {
 public:
  freelist(uint32_t batch_count) 
      : block_allocator_(batch_count, sizeof(T)) {}
  freelist(uint32_t batch_count, uint32_t item_size)
      : block_allocator_(batch_count, std::max(item_size, (uint32_t)sizeof(T))) {}

  T* alloc(uint64_t* key) {
    *key = block_allocator_.alloc_key();
    uint8_t* p = block_allocator_.get_key(*key);
    if (p) __detail__::construct_traits<T>::construct((T*)p);
    return (T*)p;  
  }

  template<typename A1>
  T* alloc(const A1& arg1, uint64_t* key) {
    *key = block_allocator_.alloc_key();
    uint8_t* p = block_allocator_.get_key(*key);
    if (p) __detail__::construct_traits<T>::construct((T*)p, arg1);
    return (T*)p;  
  }

  template<typename A1, typename A2>
  T* alloc(const A1& arg1, const A2& arg2, uint64_t* key) {
    *key = block_allocator_.alloc_key();
    uint8_t* p = block_allocator_.get_key(*key);
    if (p) __detail__::construct_traits<T>::construct((T*)p, arg1, arg2);
    return (T*)p;  
  }

  template<typename A1, typename A2, typename A3>
  T* alloc(const A1& arg1, const A2& arg2, const A3& arg3, uint64_t* key) {
    *key = block_allocator_.alloc_key();
    uint8_t* p = block_allocator_.get_key(*key);
    if (p) __detail__::construct_traits<T>::construct((T*)p, arg1, arg2, arg3);
    return (T*)p;  
  }

  template<typename Y>
  Y* alloc_with(uint64_t* key) {
    assert(sizeof(Y) <= block_allocator_.block_size());
    *key = block_allocator_.alloc_key();
    uint8_t* p = block_allocator_.get_key(*key);
    if (p) __detail__::construct_traits<Y>::construct((Y*)p);
    return (Y*)p;  
  }

  template<typename Y, typename A1>
  Y* alloc_with(const A1& arg1, uint64_t* key) {
    assert(sizeof(Y) <= block_allocator_.block_size());
    *key = block_allocator_.alloc_key();
    uint8_t* p = block_allocator_.get_key(*key);
    if (p) __detail__::construct_traits<Y>::construct((Y*)p, arg1);
    return (Y*)p;  
  }

  template<typename Y, typename A1, typename A2>
  Y* alloc_with(const A1& arg1, const A2& arg2, uint64_t* key) {
    assert(sizeof(Y) <= block_allocator_.block_size());
    *key = block_allocator_.alloc_key();
    uint8_t* p = block_allocator_.get_key(*key);
    if (p) __detail__::construct_traits<Y>::construct((Y*)p, arg1, arg2);
    return (Y*)p;  
  }

  template<typename Y, typename A1, typename A2, typename A3>
  Y* alloc_with(const A1& arg1, const A2& arg2, const A3& arg3, uint64_t* key) {
    assert(sizeof(Y) <= block_allocator_.block_size());
    *key = block_allocator_.alloc_key();
    uint8_t* p = block_allocator_.get_key(*key);
    if (p) __detail__::construct_traits<Y>::construct((Y*)p, arg1, arg2, arg3);
    return (Y*)p;  
  }

  bool destroy(T* p) {
    __detail__::construct_traits<T>::destruct(p);
    return block_allocator_.dealloc((uint8_t*)p);
  }

  bool destroy_key(uint64_t key) {
    uint8_t* p = block_allocator_.get_key(key);
    if (p) {
      __detail__::construct_traits<T>::destruct((T*)p);
      return block_allocator_.dealloc_key(key);
    } else {
      return false;
    }
  }

  template<typename Y = T>
  static Y* get_key(uint64_t key) {
    return (Y*)block_allocator::get_key(key);
  }

 private:
  block_allocator block_allocator_;
};

template<typename T>
class freelist_ptr {
 public:
  freelist_ptr() : key_(0) {
  }

  explicit freelist_ptr(uint64_t key) 
      : key_(key) {
  }

  template<typename Y>
  freelist_ptr(const freelist_ptr<Y>& other) 
      : key_(other.key_) {
  }

  template<typename Y>
  freelist_ptr<T>& operator=(const freelist_ptr<Y>& other) {
    key_ = other.key_;
    return *this;
  }

  template<typename Y>  
  bool operator == (const freelist_ptr<Y>& other) {
    return this.key_ == other.key_;
  }

  template<typename Y>  
  bool operator != (const freelist_ptr<Y>& other) {
    return this.key_ != other.key_;
  }

  operator bool() {
    return freelist<T>::get_key(key_) != NULL;
  }
 private:
  uint64_t key_;
};

} //namespace lockfree
} //namespace minotaur

#endif // _MINOTAUR_LOCKFREE_FREELIST_HPP_

