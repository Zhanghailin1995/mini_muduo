#ifndef MUDUO_NET_EVENTLOOP_H
#define MUDUO_NET_EVENTLOOP_H

#include <sys/types.h>
#include <memory>
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
  EventLoop();
  ~EventLoop();

  void Loop();

  void Quit();

  TimerId Schedule(const TimerCallback& cb, Timestamp when);

  TimerId ScheduleDelay(const TimerCallback& cb, double delay);

  TimerId ScheduleAtFixRate(const TimerCallback& cb, double interval);

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
  const pid_t thread_id_;
  Timestamp poll_return_time_;
  std::unique_ptr<class EPoller> poller_;
  std::unique_ptr<class TimerQueue> timer_queue_;
  std::vector<Channel*> active_channels_;
};

}  // namespace muduo

#endif  // MUDUO_NET_EVENTLOOP_H