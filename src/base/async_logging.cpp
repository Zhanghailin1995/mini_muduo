#include "async_logging.h"
#include "log_file.h"
#include "timestamp.h"
#include "utils.h"

#include <chrono>
#include <cstdio>
#include <functional>
#include <memory>
#include <utility>

using namespace muduo;  // NOLINT

AsyncLogging::AsyncLogging(std::string basename, off_t roll_size, int flush_interval)
    : flush_interval_(flush_interval),
      running_(false),
      basename_(std::move(basename)),
      roll_size_(roll_size),
      thread_([this] { ThreadFunc(); }, "Logging"),
      latch_(1),
      current_buffer_(std::make_unique<Buffer>()),
      next_buffer_(std::make_unique<Buffer>()) {
  current_buffer_->Bzero();
  next_buffer_->Bzero();
  buffers_.reserve(16);
}

void AsyncLogging::Append(const char* logline, int len) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (current_buffer_->Available() > len) {
    current_buffer_->Append(logline, static_cast<size_t>(len));
  } else {
    buffers_.push_back(std::move(current_buffer_));
    if (next_buffer_) {
      current_buffer_ = std::move(next_buffer_);
    } else {
      current_buffer_ = std::make_unique<Buffer>();
    }
    current_buffer_->Append(logline, static_cast<size_t>(len));
    cond_.notify_one();
  }
}

string ToFormattedString(int64_t micro_seconds_since_epoch, bool show_microseconds) {
  char buf[64] = {0};
  time_t seconds =
      static_cast<time_t>(micro_seconds_since_epoch / Timestamp::K_MICRO_SECONDS_PER_SECOND);
  struct tm tm_time{};
  gmtime_r(&seconds, &tm_time);

  if (show_microseconds) {
    int microseconds =
        static_cast<int>(micro_seconds_since_epoch % Timestamp::K_MICRO_SECONDS_PER_SECOND);
    snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d", tm_time.tm_year + 1900,
             tm_time.tm_mon + 1, tm_time.tm_mday, tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
             microseconds);
  } else {
    snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d", tm_time.tm_year + 1900,
             tm_time.tm_mon + 1, tm_time.tm_mday, tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
  }
  return buf;
}

void AsyncLogging::ThreadFunc() {
  assert(running_ == true);
  latch_.CountDown();
  LogFile output(basename_, static_cast<size_t>(roll_size_), false);
  // Prepare two empty buffers.
  BufferPtr new_buffer1(std::make_unique<Buffer>());
  BufferPtr new_buffer2(std::make_unique<Buffer>());
  new_buffer1->Bzero();
  new_buffer2->Bzero();
  BufferVector buffers_to_write;
  buffers_to_write.reserve(16);
  while (running_) {
    assert(new_buffer1 && new_buffer1->Length() == 0);
    assert(new_buffer2 && new_buffer2->Length() == 0);
    assert(buffers_to_write.empty());
    {
      std::unique_lock<std::mutex> lock(mutex_);
      // Wait until the buffer vector is not empty or timeout.
      if (buffers_.empty()) {
        cond_.wait_for(lock, std::chrono::seconds(flush_interval_));
      }
      buffers_.push_back(std::move(current_buffer_));
      current_buffer_ = std::move(new_buffer1);
      buffers_to_write.swap(buffers_);
      if (!next_buffer_) {
        next_buffer_ = std::move(new_buffer2);
      }
    }
    assert(!buffers_to_write.empty());
    if (buffers_to_write.size() > 25) {
      char buf[256];
      snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
               ToFormattedString(std::chrono::duration_cast<std::chrono::microseconds>(
                                     std::chrono::system_clock::now().time_since_epoch())
                                     .count(),
                                 true)
                   .c_str(),
               buffers_to_write.size() - 2);
      fputs(buf, stderr);
      output.Append(buf, static_cast<int>(strlen(buf)));
      buffers_to_write.erase(buffers_to_write.begin() + 2, buffers_to_write.end());
    }
    for (const auto& buffer : buffers_to_write) {
      output.Append(buffer->Data(), buffer->Length());
    }
    if (buffers_to_write.size() > 2) {
      buffers_to_write.resize(2);
    }
    if (!new_buffer1) {
      assert(!buffers_to_write.empty());
      new_buffer1 = std::move(buffers_to_write.back());
      buffers_to_write.pop_back();
      new_buffer1->Reset();
    }
    if (!new_buffer2) {
      assert(!buffers_to_write.empty());
      new_buffer2 = std::move(buffers_to_write.back());
      buffers_to_write.pop_back();
      new_buffer2->Reset();
    }
    buffers_to_write.clear();
    output.Flush();
  }
  output.Flush();
}
