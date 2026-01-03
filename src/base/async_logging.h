#ifndef MUDUO_BASE_ASYNC_LOGGING_H
#define MUDUO_BASE_ASYNC_LOGGING_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include "blocking_queue.h"
#include "bounded_blocking_queue.h"
#include "count_down_latch.h"
#include "log_stream.h"

#include <functional>
#include <memory>
#include <thread>
#include <vector>

namespace muduo {
class AsyncLogging : NonCopyable {
 public:
  AsyncLogging(std::string basename, off_t roll_size, int flush_interval = 3);
  ~AsyncLogging() {
    if (running_) {
      Stop();
    }
  }

  void Append(const char *logline, int len);

  void Start() {
    running_ = true;
    // thread_ = std::thread(&AsyncLogging::ThreadFunc, this);
    thread_ = std::thread([this] { this->ThreadFunc(); });
    latch_.Wait();
  }

  void Stop() {
    running_ = false;
    cond_.notify_all();
    if (thread_.joinable()) {
      thread_.join();
    }
  }

 private:
  void ThreadFunc();

  using Buffer = detail::FixedBuffer<detail::K_LARGE_BUFFER>;
  using BufferVector = std::vector<std::unique_ptr<Buffer>>;
  using BufferPtr = std::unique_ptr<Buffer>;

  const int flush_interval_;
  std::atomic<bool> running_;
  const std::string basename_;
  const off_t roll_size_;
  std::thread thread_;
  std::mutex mutex_;
  CountDownLatch latch_;
  std::condition_variable cond_;
  BufferPtr current_buffer_;
  BufferPtr next_buffer_;
  BufferVector buffers_;
};
}  // namespace muduo

#endif