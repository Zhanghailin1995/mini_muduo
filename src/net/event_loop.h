#ifndef MUDUO_NET_EVENTLOOP_H
#define MUDUO_NET_EVENTLOOP_H

#include <sys/types.h>
#include <memory>
#include <mutex>
#include <vector>
#include "src/base/current_thread.h"
#include "src/base/noncopyable.h"
#include "src/net/callbacks.h"
#include "src/net/timer_id.h"

namespace muduo {

class Channel;
class EPoller;
class TimerQueue;

class EventLoop : NonCopyable {
 public:
  using Functor = std::function<void()>;
  EventLoop();
  ~EventLoop();

  void Loop();

  void Quit();

  Timestamp PollReturnTime() const { return poll_return_time_; }

  // Runs callback immediately if in loop thread,
  // or queues callback in loop thread.
  // safe to call from other threads.
  void Execute(Functor cb);

  // Queues callback in loop thread.
  // safe to call from other threads.
  // the difference between Submit() and Execute() is that
  // Submit() always queues the callback.
  // User should prefer Execute(), unless they know what they are doing.
  void Submit(Functor cb);

  void WakeupExecutor();

  void RunPendingFunctors();

  TimerId Schedule(TimerCallback cb, Timestamp when);

  TimerId ScheduleDelay(TimerCallback cb, double delay);

  TimerId ScheduleAtFixRate(TimerCallback cb, double interval);

  void UpdateChannel(Channel* channel);

  void AssertInLoopThread() {
    if (!IsInLoopThread()) {
      // abort
      AbortNotInLoopThread();
    }
  }

  bool IsInLoopThread() const { return thread_id_ == current_thread::Tid(); }

 private:
  void AbortNotInLoopThread();
  bool looping_;
  bool quit_;
  bool running_pending_functors_;
  const pid_t thread_id_;
  Timestamp poll_return_time_;
  std::unique_ptr<class EPoller> poller_;
  std::unique_ptr<class TimerQueue> timer_queue_;
  std::vector<Channel*> active_channels_;
  int wakeup_fd_;
  std::unique_ptr<class Channel> wakeup_channel_;
  std::mutex mutex_;
  std::vector<Functor> pending_functors_;
};

}  // namespace muduo

#endif  // MUDUO_NET_EVENTLOOP_H