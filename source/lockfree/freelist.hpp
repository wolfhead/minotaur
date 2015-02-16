#ifndef _MINOTAUR_LOCKFREE_FREELIST_HPP_
#define _MINOTAUR_LOCKFREE_FREELIST_HPP_
/**
  @file freelist.hpp
  @author Wolfhead
*/
#include <algorithm>
#include <atomic>
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
    alloc_triats<std::is_pod<T>::value>::construct(buffer, arg1. arg2. arg3);
  }

  static void destruct(T* buffer) {
    alloc_triats<std::is_pod<T>::value>::destruct(buffer);
  }
};

} //namespace __detail__


template<class T>
class fix_sized_freelist {
 public:
  struct tagged_node;
  typedef tagged_ptr<T> tagged_handle_ptr;
  typedef tagged_ptr<tagged_node> tagged_node_ptr;

  // when the tagged node is in the freelist
  // member next point to the next free node
  // when the tagged node is allocated
  // member next hold the value of current tagged pointer
  struct tagged_node {
    uint64_t next;
    uint8_t data[0];
  };

  fix_sized_freelist(uint32_t item_count) 
      : item_count_(item_count)
      , item_size_(get_item_size()) {
    mem_buffer_ = alloc_mem_buffer(item_count_, item_size_);
    init_free_list_on_mem_buffer(mem_buffer_);
  }

  T* alloc() {
    uint64_t key = alloc_key();
    return get_key(key);
  }

  bool dealloc(T* p) {
    tagged_node* node = (tagged_node*)((uint8_t*)p - sizeof(uint64_t));
    return dealloc_key(node->next);
  }

  bool dealloc_key(uint64_t key) {
    tagged_node_ptr node(key);
    if (!check_node(node)) return false;
    __detail__::construct_traits<T>::destruct((T*)node->data);
    dealloc_node(node);
    return true;
  }

  uint64_t alloc_key() {
    tagged_node_ptr node = alloc_node();
    if (!node) return 0;
    __detail__::construct_traits<T>::construct((T*)node->data);
    return node.get_raw();
  }

  T* get_key(uint64_t key) {
    if (key == 0) return NULL;
    tagged_node_ptr node(key);
    if (!check_node(node)) return NULL;
    return (T*)node->data;
  } 

 private:

  uint32_t get_item_size() {
    return sizeof(tagged_node) + sizeof(T);
  }

  char* alloc_mem_buffer(uint32_t item_count, uint32_t item_size) {
    return new char[item_size * item_count];
  }

  void destruct_mem_buffer(char* p) {
    delete [] p;
  }

  void init_free_list_on_mem_buffer(char* buffer) {
    for (uint32_t i = 0; i != item_count_; ++i) {
      tagged_node* node = (tagged_node*)(mem_buffer_ + i * item_size_);
      if (i != item_count_ - 1) {
        node->next = tagged_node_ptr(get_tagged_node_by_index(i + 1), 0).get_raw();
      } else {
        node->next = tagged_node_ptr(0, 0);
      }
    }

    head_ = tagged_node_ptr(get_tagged_node_by_index(0), 0);
  }

  tagged_node_ptr alloc_node() {
    tagged_node_ptr old_head;

    do {
      old_head = head_.load(std::memory_order_acquire);
      if (!old_head) {
        return tagged_node_ptr();
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

  bool check_node(tagged_node_ptr node) {
    return node->next == node.get_raw();
  }

  tagged_node* get_tagged_node_by_index(uint32_t index) {
    return (tagged_node*)(mem_buffer_ + index * item_size_);
  }

  uint32_t item_count_;
  uint32_t item_size_;
  char* mem_buffer_;

  std::atomic<tagged_node_ptr> head_;

 public:

  template<typename A1>
  T* alloc(const A1& arg1) {
    uint64_t key = alloc_key(arg1);
    return get_key(key);
  }

  template<typename A1, typename A2>
  T* alloc(const A1& arg1, const A2& arg2) {
    uint64_t key = alloc_key(arg1, arg2);
    return get_key(key);
  }

  template<typename A1, typename A2, typename A3>
  T* alloc(const A1& arg1, const A2& arg2, const A3& arg3) {
    uint64_t key = alloc_key(arg1, arg2, arg3);
    return get_key(key);
  }

  template<typename A1>
  uint64_t alloc_key(const A1& arg1) {
    tagged_node_ptr node = alloc_node();
    if (!node) return 0;
    __detail__::construct_traits<T>::construct((T*)node->data, arg1);
    return node.get_raw();
  }

  template<typename A1, typename A2>
  uint64_t alloc_key(const A1& arg1, const A2& arg2) {
    tagged_node_ptr node = alloc_node();
    if (!node) return 0;
    __detail__::construct_traits<T>::construct((T*)node->data, arg1, arg2);
    return node.get_raw();
  }

  template<typename A1, typename A2, typename A3>
  uint64_t alloc_key(const A1& arg1, const A2& arg2, const A3& arg3) {
    tagged_node_ptr node = alloc_node();
    if (!node) return 0;
    __detail__::construct_traits<T>::construct(
        (T*)node->data, arg1, arg2, arg3);
    return node.get_raw();
  }
};

} //namespace lockfree
} //namespace minotaur

#endif // _MINOTAUR_LOCKFREE_FREELIST_HPP_

