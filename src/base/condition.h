#ifndef MUDUO_BASE_CONDITION_H
#define MUDUO_BASE_CONDITION_H

#include "mutex.h"

namespace muduo {
class Condition : NonCopyable {
 public:
  explicit Condition(MutexLock &mutex) : mutex_(mutex) { pthread_cond_init(&pcond_, nullptr); }
  ~Condition() { pthread_cond_destroy(&pcond_); }

  void Wait() {
    // UnassignGuard is a helper class to unassign the mutex holder during wait
    // when thread call pthread_cond_wait
    // 1. atomically release the mutex
    // 2. thread goes to waiting state
    // 3. when thread is awakened, re-acquire the mutex
    // during this period, the mutex is not held by any thread，but the holder_ is still set
    // so we need to unassign it before wait and re-assign it after wait
    MutexLock::UnassignGuard ug(mutex_);
    pthread_cond_wait(&pcond_, mutex_.GetPthreadMutex());
  }

  bool WaitForSeconds(double seconds);

  void Notify() { pthread_cond_signal(&pcond_); }

  void NotifyAll() { pthread_cond_broadcast(&pcond_); }

 private:
  MutexLock &mutex_;
  pthread_cond_t pcond_;
};
}  // namespace muduo

#endif  // MUDUO_BASE_CONDITION_H