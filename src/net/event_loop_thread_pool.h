#ifndef MUDUO_NET_EVENTLOOP_THREAD_POOL_H
#define MUDUO_NET_EVENTLOOP_THREAD_POOL_H

#include "src/base/noncopyable.h"
#include "src/base/thread.h"
#include <memory>
#include <vector>

namespace muduo {
class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : NonCopyable {
 public:
  EventLoopThreadPool(EventLoop* base_loop);
  ~EventLoopThreadPool();

  void SetThreadCount(int num_threads) { num_threads_ = num_threads; }

  void Start();

  EventLoop* GetNextLoop();

 private:
  EventLoop* base_loop_;
  std::vector<std::unique_ptr<EventLoopThread>> threads_;
  std::vector<EventLoop*> loops_;
  int num_threads_;
  int next_loop_index_;
  bool started_;
};

}  // namespace muduo

#endif  // MUDUO_NET_EVENTLOOP_THREAD_POOL_H
