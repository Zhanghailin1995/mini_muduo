#define __STDC_LIMIT_MACROS

#include "src/net/timer_queue.h"
#include "src/base/logging.h"
#include "src/base/utils.h"
#include "src/net/event_loop.h"
#include "src/net/timer.h"
#include "src/net/timer_id.h"

#include <sys/timerfd.h>
#include <functional>

namespace muduo {

namespace detail {
int CreateTimerfd() {
  int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  if (timerfd < 0) {
    LOG_SYSFATAL << "Failed in timerfd_create";
  }
  return timerfd;
}

struct timespec DurationSinceNow(Timestamp when) {
  int64_t microseconds = when.MicroSecondsSinceEpoch() - Timestamp::Now().MicroSecondsSinceEpoch();
  if (microseconds < 100) {
    microseconds = 100;
  }
  struct timespec ts{};
  ts.tv_sec = static_cast<time_t>(microseconds / Timestamp::K_MICRO_SECONDS_PER_SECOND);
  ts.tv_nsec =
      static_cast<long>((microseconds % Timestamp::K_MICRO_SECONDS_PER_SECOND) * 1000);  // NOLINT
  return ts;
}

void ReadTimerFd(int timer_fd, Timestamp now) {
  uint64_t howmany;
  ssize_t n = ::read(timer_fd, &howmany, sizeof howmany);
  LOG_TRACE << "TimerQueue::ReadTimerFd() " << howmany << " at " << now.ToFormattedString();
  if (n != sizeof howmany) {
    LOG_ERROR << "TimerQueue::ReadTimerFd() reads " << n << " bytes instead of 8";
  }
}

void ResetTimerFd(int timer_fd, Timestamp expiration) {
  struct itimerspec new_value{};
  struct itimerspec old_value{};
  memset(&new_value, 0, sizeof new_value);
  memset(&old_value, 0, sizeof old_value);

  new_value.it_value = DurationSinceNow(expiration);
  int ret = ::timerfd_settime(timer_fd, 0, &new_value, &old_value);
  if (ret) {
    LOG_SYSERR << "timerfd_settime()";
  }
}
}  // namespace detail

}  // namespace muduo

using namespace muduo;

using namespace muduo::detail;

TimerQueue::TimerQueue(EventLoop* loop)
    : loop_(loop), timer_fd_(CreateTimerfd()), timer_fd_channel_(loop, timer_fd_), timers_() {
  timer_fd_channel_.SetReadCallback(
      std::bind(&TimerQueue::HandleTimeout, this, std::placeholders::_1));
  timer_fd_channel_.EnableReading();
}

TimerQueue::~TimerQueue() {
  ::close(timer_fd_);
  for (const Entry& timer : timers_) {
    delete timer.second;
  }
}

TimerId TimerQueue::AddTimer(TimerCallback cb, Timestamp when, double interval) {
  Timer* timer = new Timer(std::move(cb), when, interval);
  loop_->Execute(std::bind(&TimerQueue::AddTimerInEventExecutor, this, timer));
  return TimerId(timer);
}

void TimerQueue::AddTimerInEventExecutor(Timer* timer) {
  loop_->AssertInLoopThread();
  bool earliest_changed = Insert(timer);
  if (earliest_changed) {
    ResetTimerFd(timer_fd_, timer->Expiration());
  }
}

void TimerQueue::HandleTimeout(Timestamp receive_time) {
  loop_->AssertInLoopThread();
  Timestamp now(receive_time);
  ReadTimerFd(timer_fd_, now);

  std::vector<Entry> expired = GetExpired(now);

  for (const Entry& entry : expired) {
    entry.second->Run();
  }

  Reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::GetExpired(Timestamp now) {
  std::vector<Entry> expired;
  Entry sentry = std::make_pair(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
  auto end = timers_.lower_bound(sentry);
  std::copy(timers_.begin(), end, std::back_inserter(expired));
  timers_.erase(timers_.begin(), end);
  return expired;
}

void TimerQueue::Reset(const std::vector<Entry>& expired, Timestamp now) {
  for (const Entry& entry : expired) {
    Timer* timer = entry.second;
    if (timer->Repeat()) {
      LOG_INFO << "TimerQueue::Reset() repeat timer";
      timer->Restart(Timestamp(now));
      Insert(timer);
    } else {
      delete timer;
    }
  }

  if (!timers_.empty()) {
    Timestamp next_expire = timers_.begin()->second->Expiration();
    if (next_expire.Valid()) {
      ResetTimerFd(timer_fd_, next_expire);
    }
  }
}

bool TimerQueue::Insert(Timer* timer) {
  loop_->AssertInLoopThread();
  bool earliest_changed = false;
  Timestamp when = timer->Expiration();
  if (timers_.empty() || when < timers_.begin()->first) {
    earliest_changed = true;
  }
  timers_.insert(std::make_pair(when, timer));
  return earliest_changed;
}