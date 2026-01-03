#include "count_down_latch.h"

using namespace muduo;  // NOLINT

void CountDownLatch::Wait() {
  std::unique_lock<std::mutex> lock(mutex_);
  while (count_ > 0) {
    cond_.wait(lock);
  }
}

void CountDownLatch::CountDown() {
  std::lock_guard<std::mutex> lock(mutex_);
  --count_;
  if (count_ == 0) {
    cond_.notify_all();
  }
}

int CountDownLatch::GetCount() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return count_;
}