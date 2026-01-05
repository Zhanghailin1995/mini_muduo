//
// Created by hailin on 11/14/22.
//
#include "src/net/timer.h"

using namespace muduo;  // NOLINT

void Timer::Restart(Timestamp now) {
  if (repeat_) {
    expiration_ = AddTime(now, interval_);
  } else {
    expiration_ = Timestamp::Invalid();
  }
}