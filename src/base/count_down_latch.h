#ifndef MUDUO_BASE_COUNT_DOWN_LATCH_H
#define MUDUO_BASE_COUNT_DOWN_LATCH_H

#include "noncopyable.h"

#include <condition_variable>
#include <mutex>

namespace muduo {
class CountDownLatch : NonCopyable {
 public:
  explicit CountDownLatch(int count) : count_(count) {};

  void Wait();

  void CountDown();
  int GetCount() const;

 private:
  mutable std::mutex mutex_;
  std::condition_variable cond_;
  int count_;
};
}  // namespace muduo

#endif  // MUDUO_BASE_COUNT_DOWN_LATCH_H