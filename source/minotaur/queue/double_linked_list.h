#ifndef _MINOTAUR_DOUBLE_LINKED_LIST_H_
#define _MINOTAUR_DOUBLE_LINKED_LIST_H_
/**
 * @file double_linked_list.h
 * @author Wolfhead
 */
#include <unistd.h>
#include <stdint.h>

namespace ade {

template<typename T>
struct DoubleLinkedListTraits {
  inline static void set_prev(T* self, T* prev) {self->prev_ = prev;} 
  inline static T*   get_prev(T* self) {return self->prev_;}
  inline static void set_next(T* self, T* next) {self->next_ = next;} 
  inline static T*   get_next(T* self) {return self->next_;} 
};

template<typename T>
class DoubleLinkedList {
 public:
  typedef DoubleLinkedListTraits<T> Traits; 

  DoubleLinkedList() 
      : front_(NULL)
      , tail_(NULL)
      , size_(0) {
  }

  DoubleLinkedList(const DoubleLinkedList& other)
      : front_(other.front_)
      , tail_(other.tail_)
      , size_(other.size_) {
  }

  DoubleLinkedList& operator = (const DoubleLinkedList& other) {
    front_ = other.front_;
    tail_ = other.tail_;
    size_ = other.size_;
    return *this;
  }

  inline T* front() {return front_;}
  inline T* tail() {return tail_;}

  DoubleLinkedList& push_front(T* p) {
    Traits::set_prev(p, NULL);
    Traits::set_next(p, front_);
    if (front_) {
      Traits::set_prev(front_, p);
      front_ = p;
    } else {
      front_ = tail_ = p;
    }
    ++size_;
    return *this;
  }

  void pop_front() {
    T* old_front = front_;
    T* next = Traits::get_next(front_);

    Traits::set_prev(old_front, NULL);
    Traits::set_next(old_front, NULL);

    if (next) {
      Traits::set_prev(next, NULL);
      front_ = next;
    } else {
      front_ = tail_ = NULL;
    }
    --size_;
  }

  DoubleLinkedList& push_back(T* p) {
    Traits::set_prev(p, tail_);
    Traits::set_next(p, NULL);
    if (tail_) {
      Traits::set_next(tail_, p);
      tail_ = p;
    } else {
      front_ = tail_ = p;
    }
    ++size_;
    return *this;
  }

  void pop_tail() {
    T* old_tail = tail_;
    T* prev = Traits::get_prev(tail_);
    
    Traits::set_prev(old_tail, NULL);
    Traits::set_next(old_tail, NULL);

    if (prev) {
      Traits::set_next(prev, NULL);
      tail_ = prev;
    } else {
      front_ = tail_ = NULL;
    }
    --size_;
  }

  void erase(T* p) {
    T* prev = Traits::get_prev(p);
    T* next = Traits::get_next(p);
    if (prev) {
      Traits::set_next(prev, next);
    } else {
      front_ = next;
    }

    if (next) {
      Traits::set_prev(next, prev);
    } else {
      tail_ = prev;
    }
    --size_;

    Traits::set_prev(p, NULL);
    Traits::set_next(p, NULL);
  }

  void clear() {
    front_ = tail_ = NULL;
    size_ = 0;
  }

  inline uint32_t size() const {return size_;}

  inline bool empty() const {return size() == 0;}

 private:
  T* front_;
  T* tail_;
  uint32_t size_;
};

} //namespace ade

#endif // _MINOTAUR_DOUBLE_LINKED_LIST_H_
