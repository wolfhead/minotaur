#ifndef _MINOTAUR_STATIC_LIST_POOL_H_
#define _MINOTAUR_STATIC_LIST_POOL_H_
/**
  @file static_list.h
  @author Wolfhead
*/
#include <stdio.h>
namespace minotaur {

#define CAS(a_ptr, a_oldVal, a_newVal) \
__sync_bool_compare_and_swap(a_ptr, a_oldVal, a_newVal)

template<typename T>
class StaticListPool {
 public:
  typedef int32_t SizeType;
  typedef int32_t LinkType;

  StaticListPool(SizeType size) {
    size_ = size;
    list_ = new Holder[size_];

    for (SizeType i = 0; i != size_; ++i) {
      list_[i].next = i + 1;
    }

    if (!size_) {
      list_[size_ - 1].next = -1;
      head_ = -1;
    } else {
      head_ = 0;
    }
  }

  ~StaticListPool() {
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
    } while (!CAS(&head_, head, index));

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
    } while (!CAS(&head_, head, next_head));

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
    LinkType next;
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
  LinkType  head_;
};

template<typename T>
class List {
 public:
  typedef int32_t SizeType;
  typedef int32_t LinkType;

  List(SizeType size) {
    size_ = size;
    list_ = new Holder[size_];

    for (SizeType i = 0; i != size_; ++i) {
      list_[i].next = i + 1;
    }

    if (!size_) {
      list_[size_ - 1].next = -1;
      head_ = -1;
    } else {
      head_ = 0;
    }
  }

  ~List() {
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
    boost::mutex::scoped_lock scoped_lock(swap_mutex_);
    head = head_;
    list_[index].next = head;
    head_ = head;

    return true;
  }

  bool Alloc(T** value) {
    LinkType head;
    LinkType next_head;

    boost::mutex::scoped_lock scoped_lock(swap_mutex_);

    head = head_;
    if (head < 0) {
      return false;
    }
    next_head = list_[head].next;
    head_ = next_head;
    *value = &list_[head].item;
    return true;
  }


 private:
  struct Holder {
    LinkType next;
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
  LinkType  head_;

  boost::mutex swap_mutex_;
};


} //namespace minotaur

#endif // _MINOTAUR_STATIC_LIST_H_
