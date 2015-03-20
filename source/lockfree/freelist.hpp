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
#include <iostream>
#include <type_traits>
#include <typeinfo>
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

  template<typename T, typename... Args>
  static void construct(T* buffer, const Args&... args) {
    new (buffer) T(args...);
  }

  template<typename T>
  static void destruct(T*) {}
};

template<>
struct alloc_triats<false> {
  template<typename T, typename... Args>
  static void construct(T* buffer, const Args&... args) {
    new (buffer) T(args...);
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

  template<typename... Args>
  static void construct(T* buffer, const Args&... args) {
    alloc_triats<std::is_pod<T>::value>::construct(buffer, args...);
  }

  static void destruct(T* buffer) {
    alloc_triats<std::is_pod<T>::value>::destruct(buffer);
  }
};

} //namespace __detail__


/**
 *
 * @thread-safe: 
 *  dealloc and alloc can work concurruntly in different threads
 *  dealloc and get_key to the same key
 *  dealloc and dealloc to the same key 
 *  is not thread safe
 */
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
    node->next = 0;
    dealloc_node(node);
    return true;
  }

  static uint8_t* get_key(uint64_t key) {
    if (key == 0) return NULL;
    tagged_node_ptr node(key);
    if (!check_node(node)) return NULL;
    return node->data;
  } 

  static bool valid_key(uint64_t key) {
    return check_node(tagged_node_ptr(key));
  }

  static bool valid(uint8_t* p) {
    tagged_node* node = (tagged_node*)((uint8_t*)p - sizeof(uint64_t));
    return valid_key(node->next);
  }

  template<typename T>
  void print_stat(std::ostream& os) {
    uint32_t alloc_count = 0;
    uint32_t free_count = 0;
    for (uint8_t* p : pool_) {
      get_stat_on_mem_buffer(p, &alloc_count, &free_count);
    }

    os << "freelist<" << typeid(T).name() << ">"
      << ", block:" << block_size_
      << ", alloc:" << alloc_count 
      << ", free:" << free_count << std::endl;
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
      tagged_node_ptr p(node, i);
      dealloc_node(p);
    }
  }

  void get_stat_on_mem_buffer(uint8_t* buffer, uint32_t* alloc_count, uint32_t* free_count) {
    for (uint32_t i = 0; i != batch_count_; ++i) {
      tagged_node* node = (tagged_node*)(buffer + i * block_size_);
      if (tagged_node_ptr::extract_ptr(node->next) == node) {
        ++(*alloc_count);
      } else {
        ++(*free_count);
      }
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
#ifdef MINOTAUR_MEM_CHECK
  ~freelist() {block_allocator_.print_stat<T>(std::cout);}
#endif
    
  template<typename... Args>
  T* alloc(uint64_t* key, const Args&... args) {
    *key = block_allocator_.alloc_key();
    uint8_t* p = block_allocator_.get_key(*key);
    if (p) __detail__::construct_traits<T>::construct((T*)p, args...);
    return (T*)p;  
  }

  template<typename Y, typename... Args>
  Y* alloc_with(uint64_t* key, const Args&... args) {
    assert(sizeof(Y) <= block_allocator_.block_size());
    *key = block_allocator_.alloc_key();
    uint8_t* p = block_allocator_.get_key(*key);
    if (p) __detail__::construct_traits<Y>::construct((Y*)p, args...);
    return (Y*)p;  
  }

  bool destroy(T* p) {
    if (block_allocator::valid((uint8_t*)p)) {
      __detail__::construct_traits<T>::destruct(p);
    }
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

