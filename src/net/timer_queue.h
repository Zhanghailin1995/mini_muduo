#ifndef MUDUO_NET_TIMER_QUEUE_H
#define MUDUO_NET_TIMER_QUEUE_H

#include <set>
#include <vector>

#include "src/base/noncopyable.h"
#include "src/base/timestamp.h"
#include "src/net/callbacks.h"
#include "src/net/channel.h"

namespace muduo {
class EventLoop;
class Timer;
class TimerId;

class TimerQueue : NonCopyable {
 public:
  explicit TimerQueue(EventLoop* loop);
  ~TimerQueue();

  TimerId AddTimer(TimerCallback cb, Timestamp when, double interval);

 private:
  using Entry = std::pair<Timestamp, Timer*>;
  using TimerSet = std::set<Entry>;

  void AddTimerInEventExecutor(Timer* timer);

  void HandleTimeout();
  std::vector<Entry> GetExpired(Timestamp now);
  void Reset(const std::vector<Entry>& expired, Timestamp now);

  bool Insert(Timer* timer);
  EventLoop* loop_;
  const int timer_fd_;
  Channel timer_fd_channel_;
  // Timer list sorted by expiration
  TimerSet timers_;
};

}  // namespace muduo

#endif