#ifndef _MINOTAUR_COMMON_SCOPE_GUARD_H_
#define _MINOTAUR_COMMON_SCOPE_GUARD_H_
/**
 * @file scope_gurad.h
 * @author Wolfhead
 */
#include <functional>

namespace minotaur {

class ScopeGuard {
 public:
  ScopeGuard(const std::function<void()>& functor) 
      : functor_(functor)
      , dispose_(false) {
  }

  ~ScopeGuard() {
    if (!dispose_) functor_();
  }

  void Dispose() {
    dispose_ = true;
  }

 private:
  std::function<void()> functor_;
  bool dispose_;
};

} //namespace minotaur

#endif // _MINOTAUR_COMMON_SCOPE_GUARD_H_
