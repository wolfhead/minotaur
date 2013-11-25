#ifndef MINOTAUR_LOCAL_HPP
#define MINOTAUR_LOCAL_HPP
/**
  @file local.hpp
  @author Wolfhead
*/

namespace mt {

template<typename T>
class local {
 public:
  typedef local<T> self;
  typedef T*       pointer;
  typedef T        reference;

  template<typename Y>
  explicit local(Y* p) : p_(p) {

  }

  ~local() {
    clear();
  }

  pointer operator->() const {
    return p_;
  }

  reference operator * () const {
    return * p_;
  }

  pointer get() const {
    return p_;
  }

  pointer takeover() {
    pointer t = p_;
    p_ = NULL;
    return t;
  }

  template<typename Y>
  void reset(Y* p) {
    if (p_) delete p_;
    p_ = p;
  }

  void clear() {
    if (p_) delete p_;
    p_ = NULL;
  }

 private:

  local();

  local(const self&);

  self& operator = (const self&);

  pointer p_;
};


} //namespace mt

#endif // MINOTAUR_LOCAL_HPP
