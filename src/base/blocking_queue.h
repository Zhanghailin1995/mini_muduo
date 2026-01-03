#ifndef MUDUO_BASE_BLOCKINGQUEUE_H
#define MUDUO_BASE_BLOCKINGQUEUE_H

#include <cassert>
#include <condition_variable>
#include <deque>
#include <mutex>
#include "noncopyable.h"

namespace muduo {

template <typename T>
class BlockingQueue : NonCopyable {
 public:
  BlockingQueue() : queue_() {}

  void Put(const T &x) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push_back(x);
    not_empty_.notify_all();
  }

  void Put(T &&x) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push_back(std::move(x));
    not_empty_.notify_all();
  }

  T Take() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (queue_.empty()) {
      not_empty_.wait(lock);
    }
    assert(!queue_.empty());
    T front(std::move(queue_.front()));
    queue_.pop_front();
    return front;
  }

  // drain the queue
  std::deque<T> TakeAll() {
    std::deque<T> tmp;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      tmp = std::move(queue_);
      assert(queue_.empty());
    }
    return tmp;
  }

  size_t Size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
  }

 private:
  mutable std::mutex mutex_;
  std::condition_variable not_empty_;
  std::deque<T> queue_;
};
}  // namespace muduo

#endif  // MUDUO_BASE_BLOCKINGQUEUE_H