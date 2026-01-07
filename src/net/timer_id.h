//
// Created by hailin on 11/14/22.
//

#ifndef MUDUO_NET_TIMER_ID_H
#define MUDUO_NET_TIMER_ID_H

namespace muduo {
class Timer;

/// An opaque identifier, for canceling Timer.

class TimerId {
 public:
  explicit TimerId(Timer* timer) : timer_(timer) {}

  // default copy-ctor, dtor and assignment are okay

 private:
  Timer* timer_;
};
}  // namespace muduo

#endif  // MUDUO_NET_TIMER_ID_H
