//
// Created by hailin on 11/17/22.
//

#ifndef MINI_MUDUO_EVENTLOOP_THREAD_H
#define MINI_MUDUO_EVENTLOOP_THREAD_H

#include "src/base/noncopyable.h"
#include "src/base/thread.h"

namespace muduo {
class EventLoop;

class EventLoopThread : NonCopyable {
 public:
  EventLoopThread();
  ~EventLoopThread();
  EventLoop* StartLoop();

 private:
  void ThreadFunc();

  EventLoop* loop_;
  Thread thread_;
  std::mutex mutex_;
  std::condition_variable cond_;
  bool exiting_;
};
}  // namespace muduo

#endif  // TINY_MUDUO_EVENTLOOP_THREAD_H
