#ifndef MUDUO_NET_EVENTLOOP_H
#define MUDUO_NET_EVENTLOOP_H

#include <sys/types.h>
#include <memory>
#include <vector>
#include "src/base/current_thread.h"
#include "src/base/noncopyable.h"

namespace muduo {

class Channel;
class EPoller;
class EventLoop : NonCopyable {
 public:
  EventLoop();
  ~EventLoop();

  void Loop();

  void Quit();

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
  std::unique_ptr<class EPoller> poller_;
  std::vector<Channel*> active_channels_;
};

}  // namespace muduo

#endif  // MUDUO_NET_EVENTLOOP_H