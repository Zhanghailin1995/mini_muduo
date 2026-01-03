#ifndef MUDUO_BASE_BOUNDED_BLOCKING_QUEUE_H
#define MUDUO_BASE_BOUNDED_BLOCKING_QUEUE_H

#include <boost/circular_buffer.hpp>
#include <cassert>
#include <condition_variable>
#include <mutex>
#include "noncopyable.h"

namespace muduo {
template <typename T>
class BoundedBlockingQueue : NonCopyable {
 public:
  explicit BoundedBlockingQueue(int max_size) : queue_(max_size) {}

  void Put(const T &x) {
    std::unique_lock<std::mutex> lock(mutex_);
    while (queue_.full()) {
      not_full_.wait(lock);
    }
    assert(!queue_.full());
    queue_.push_back(x);
    not_empty_.notify_all();
  }

  void Put(T &&x) {
    std::unique_lock<std::mutex> lock(mutex_);
    while (queue_.full()) {
      not_full_.wait(lock);
    }
    assert(!queue_.full());
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
    not_full_.notify_all();
    return front;
  }

  size_t Size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
  }

  size_t Capacity() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.capacity();
  }

  bool Empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
  }

  bool Full() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.full();
  }

 private:
  mutable std::mutex mutex_;
  std::condition_variable not_empty_;
  std::condition_variable not_full_;
  boost::circular_buffer<T> queue_;
};
}  // namespace muduo

#endif