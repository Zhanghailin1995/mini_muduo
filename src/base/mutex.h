// It's a not reentrant mutex lock class.
// Created by hailin on 11/16/22.

#ifndef MUDUO_BASE_MUTEX_H
#define MUDUO_BASE_MUTEX_H

#include <pthread.h>
#include <cassert>
#include "current_thread.h"
#include "noncopyable.h"

namespace muduo {
class MutexLock : NonCopyable {
 public:
  MutexLock() : holder_(0) { pthread_mutex_init(&mutex_, nullptr); }
  ~MutexLock() {
    assert(holder_ == 0);
    pthread_mutex_destroy(&mutex_);
  }

  void Lock() {
    pthread_mutex_lock(&mutex_);
    AssignHolder();
  }
  void Unlock() {
    UnassignHolder();
    pthread_mutex_unlock(&mutex_);
  }

  pthread_mutex_t *GetPthreadMutex() { return &mutex_; }

 private:
  friend class Condition;
  class UnassignGuard : NonCopyable {
   public:
    explicit UnassignGuard(MutexLock &mutex) : mutex_(mutex) { mutex_.UnassignHolder(); }
    ~UnassignGuard() { mutex_.AssignHolder(); }

   private:
    MutexLock &mutex_;
  };

  void UnassignHolder() { holder_ = 0; }
  void AssignHolder() { holder_ = current_thread::Tid(); }

  pthread_mutex_t mutex_;
  pid_t holder_;
};

class MutexLockGuard : NonCopyable {
 public:
  explicit MutexLockGuard(MutexLock &mutex) : mutex_(mutex) { mutex_.Lock(); }
  ~MutexLockGuard() { mutex_.Unlock(); }

 private:
  MutexLock &mutex_;
};

}  // namespace muduo

#define MutexLockGuard(x) static_assert(false, "Missing mutex guard variable name")

#endif