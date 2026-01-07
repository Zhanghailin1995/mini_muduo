//
// Created by hailin on 11/14/22.
//

#ifndef MUDUO_NET_TIMER_H
#define MUDUO_NET_TIMER_H

#include <utility>

#include "src/base/noncopyable.h"
#include "src/net/callbacks.h"

namespace muduo {
class Timer : NonCopyable {
 public:
  Timer(TimerCallback cb, Timestamp when, double interval)
      : callback_(std::move(cb)), expiration_(when), interval_(interval), repeat_(interval > 0.0) {}

  void Run() const { callback_(); }

  Timestamp Expiration() const { return expiration_; }

  bool Repeat() const { return repeat_; }

  void Restart(Timestamp now);

 private:
  const TimerCallback callback_;
  Timestamp expiration_;
  const double interval_;
  const bool repeat_;
};
}  // namespace muduo

#endif  // MUDUO_NET_TIMER_H
