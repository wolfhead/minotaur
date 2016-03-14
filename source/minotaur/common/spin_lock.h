#ifndef MINOTAUR_SPIN_LOCK_H
#define MINOTAUR_SPIN_LOCK_H
/**
 * @file spin_lock.h
 * @author Wolfhead
 */
#include <atomic>

namespace ade {

class spinlock {
 public:
  spinlock() {flag.clear(std::memory_order::memory_order_release);}
  void lock() {while(flag.test_and_set(std::memory_order_acquire)){};}
  void unlock() {flag.clear(std::memory_order_release);}
 private:
  std::atomic_flag flag;
};

class spinlock_scope {
 public:
  spinlock_scope(spinlock& lock) : lock_(lock) {
    lock_.lock();
  }
  ~spinlock_scope() {
    lock_.unlock();
  }

 private:
  spinlock& lock_;
};

} //namespace ade

#endif //MINOTAUR_SPIN_LOCK_H
