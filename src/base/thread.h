#ifndef TINY_MUDUO_THREAD_H
#define TINY_MUDUO_THREAD_H

#include <sys/types.h>
#include <atomic>
#include <functional>
#include <string>
#include "count_down_latch.h"

namespace muduo {

class Thread : NonCopyable {
 public:
  using ThreadFunc = std::function<void()>;

  explicit Thread(ThreadFunc, std::string name = std::string());

  ~Thread();

  void Start();
  void Join();

  bool Started() const { return started_; }
  pid_t Tid() const { return tid_; }
  const std::string &Name() const { return name_; }

  static int NumCreated() { return num_created.load(); }

 private:
  void SetDefaultName();

  bool started_;
  bool joined_;
  pthread_t pthread_id_;
  pid_t tid_;
  ThreadFunc func_;
  std::string name_;
  CountDownLatch latch_;

  static std::atomic<int32_t> num_created;
};

}  // namespace muduo

#endif  // TINY_MUDUO_THREAD_H