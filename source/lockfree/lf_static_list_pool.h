#ifndef _MINOTAUR_LF_STATIC_LIST_POOL_H_
#define _MINOTAUR_LF_STATIC_LIST_POOL_H_
/**
  @file lf_static_list.h
  @author Wolfhead
*/
#include "lf_util.h"

namespace minotaur {
namespace lockfree {

/**
  LFStaticListPool for multi-Alloc/multi-Free
*/
template<typename T>
class LFStaticListPool {
 public:
  typedef int SizeType;
  typedef int LinkType;

  LFStaticListPool(SizeType size) {
    size_ = size;
    list_ = new Holder[size_];

    for (SizeType i = 0; i != size_; ++i) {
      list_[i].next = i + 1;
    }

    if (!size_) {
      head_ = -1;
    } else {
      list_[size_ - 1].next = -1;
      head_ = 0;
    }
  }

  ~LFStaticListPool() {
    if (list_) {
      delete [] list_;
    }
  }

  bool Free(T* value) {
    LinkType index = GetIndex(value);
    if (index < 0 || index >= size_) {
      return false;
    }

    LinkType head;

    do {
       head = head_;
       list_[index].next = head;
    } while (!LF_CAS(&head_, head, index));

    return true;
  }

  bool Alloc(T** value) {
    LinkType head;
    LinkType next_head;

    do {
      head = head_;
      if (head < 0) {
        return false;
      }
      next_head = list_[head].next;
    } while (!LF_CAS(&head_, head, next_head));

    *value = &list_[head].item;
    return true;
  }

  bool At(LinkType index, T** value) {
    if (index < 0 || index >= size_) {
      return false;
    }

    *value = &list_[index].item;
    return true;
  }

 private:
  struct Holder {
    volatile LinkType next;
    T       item;
  };

  Holder* GetHolder(void* p) {
    return (Holder*)((char *)p - sizeof(LinkType));
  }

  LinkType GetIndex(void* p) {
    return GetHolder(p) - list_;
  }

  SizeType  size_;
  Holder*   list_;
  volatile  LinkType head_;
};

} //namespace lockfree
} //namespace minotaur

#endif // _MINOTAUR_LF_STATIC_LIST_POOL_H_
