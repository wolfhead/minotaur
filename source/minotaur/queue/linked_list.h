#ifndef _MINOTAUR_QUEUE_LINKED_LIST_H_
#define _MINOTAUR_QUEUE_LINKED_LIST_H_
/**
 * @file linked_list.h
 * @author Wolfhead
 */
#include <unistd.h>
#include <stdint.h>

namespace ade {

template<typename T>
struct LinkedListNode {
  T* next_;
};

template<typename T>
struct LinkedListTraits {
  inline static void set_next(T* self, T* next) {self->next_ = next;} 
  inline static T*   get_next(T* self) {return self->next_;} 
};

template<typename T>
class LinkedList {
 public:
  typedef LinkedListTraits<T> Traits; 

  LinkedList() 
      : front_(NULL)
      , tail_(NULL)
      , size_(0) {
  }

  inline T* front() const {return front_;}
  inline T* tail() const {return tail_;}

  LinkedList& push_front(T* p) {
    ++size_;
    Traits::set_next(p, front_);
    front_ = p;
    if (!tail_) {
      tail_ = p; 
    }
    return *this;
  }

  void pop_front() {
    --size_;
    T* old_front = front_;
    front_ = Traits::get_next(front_);
    Traits::set_next(old_front, NULL);
    if (!front_) {
      tail_ = NULL;
    }
  }

  LinkedList& push_back(T* p) {
    if (!tail_) {
      return push_front(p);
    } else {
      Traits::set_next(tail_, p);
      Traits::set_next(p, NULL);
      tail_ = p;
    }
    ++size_;
    return *this;
  }

  inline uint32_t size() const {return size_;}

  inline bool empty() const {return size() == 0;}

  inline void clear() {
    front_ = tail_ = NULL;
    size_ = 0;
  }

 private:
  T* front_;
  T* tail_; 
  uint32_t size_;
};

} //namespace ade

#endif //_MINOTAUR_QUEUE_LINKED_LIST_H_
