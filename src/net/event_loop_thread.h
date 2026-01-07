//
// Created by hailin on 11/17/22.
//

#ifndef MUDUO_NET_EVENTLOOP_THREAD_H
#define MUDUO_NET_EVENTLOOP_THREAD_H

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

#endif  // MUDUO_NET_EVENTLOOP_THREAD_H
