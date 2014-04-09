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

  template<typename T>
  static void destruct(T*) {}
};

template<>
struct alloc_triats<false> {
  template<typename T>
  static void construct(T* buffer) {
    new (buffer) T;
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

  struct tagged_node {
    tagged_node_ptr next;
  };

  fix_sized_freelist(uint32_t item_count) 
      : item_count_(item_count)
      , item_size_(get_item_size()) {
    mem_buffer_ = alloc_mem_buffer(item_count_, item_size_);
    init_free_list_on_mem_buffer(mem_buffer_);
  }

  T* alloc() {
    char* buffer = alloc_raw();
    if (!buffer) return NULL;
    __detail__::construct_traits<T>::construct((T*)buffer);
    return (T*)buffer;
  }

  void dealloc(T* p) {
    __detail__::construct_traits<T>::destruct(p);
    dealloc_raw((char*)p);
  }

 private:

  uint32_t get_item_size() {
    return std::max(sizeof(tagged_node), sizeof(T));
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
        node->next.set((tagged_node*)(mem_buffer_ + ((i + 1) * item_size_)), 0);
      } else {
        node->next.set(0, 0);
      }
    }

    head_ = tagged_node_ptr((tagged_node*)mem_buffer_, 0); 
  }

  char* alloc_raw() {
    tagged_node_ptr old_head = head_.load(std::memory_order_consume);

    for(;;) {
      if (!old_head.get_ptr()) {
        return 0;
      }

      tagged_node_ptr new_head(old_head->next.get_ptr(), old_head.get_tag() + 1);

      if (head_.compare_exchange_weak(old_head, new_head)) {
        return reinterpret_cast<char*>(old_head.get_ptr());
      }
    }
  }

  void dealloc_raw(char* buffer) {
    tagged_node_ptr old_head = head_.load(std::memory_order_consume);
    tagged_node * new_node = reinterpret_cast<tagged_node*>(buffer);

    for(;;) {
      tagged_node_ptr new_head(new_node, old_head.get_tag());
      new_head->next = old_head;

      if (head_.compare_exchange_weak(old_head, new_head)) {
        return;
      }
    }
  }

  uint32_t item_count_;
  uint32_t item_size_;
  char* mem_buffer_;

  std::atomic<tagged_node_ptr> head_;
};

} //namespace lockfree
} //namespace minotaur


#endif // _MINOTAUR_LOCKFREE_FREELIST_HPP_

