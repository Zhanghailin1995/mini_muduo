#include "src/net/event_loop.h"
#include "src/base/current_thread.h"
#include "src/base/logging.h"
#include "src/net/channel.h"
#include "src/net/epoller.h"
#include "src/net/timer_queue.h"

using namespace muduo;  // NOLINT

__thread EventLoop* t_loop_in_this_thread = nullptr;
const int K_POLL_TIME_MS = 10 * 1000;

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      thread_id_(current_thread::Tid()),
      poller_(std::make_unique<EPoller>(this)),
      timer_queue_(std::make_unique<TimerQueue>(this)) {
  LOG_TRACE << "EventLoop created " << this << " in thread " << thread_id_;
  if (t_loop_in_this_thread != nullptr) {
    LOG_FATAL << "Another EventLoop " << t_loop_in_this_thread << " exists in this thread "
              << thread_id_;
  } else {
    t_loop_in_this_thread = this;
  }
}

EventLoop::~EventLoop() {
  LOG_TRACE << "EventLoop " << this << " of thread " << thread_id_ << " destructs";
  assert(!looping_);
  t_loop_in_this_thread = nullptr;
}

void EventLoop::Loop() {
  assert(!looping_);
  AssertInLoopThread();
  looping_ = true;
  quit_ = false;
  LOG_TRACE << "EventLoop " << this << " start looping";

  while (!quit_) {
    active_channels_.clear();
    poll_return_time_ = poller_->Poll(K_POLL_TIME_MS, &active_channels_);
    for (Channel* channel : active_channels_) {
      channel->HandleEvent();
    }
  }

  LOG_TRACE << "EventLoop " << this << " stop looping";
  looping_ = false;
}

void EventLoop::Quit() {
  quit_ = true;
  if (!IsInLoopThread()) {
    // wake up IO thread (omitted for brevity)
  }
}

TimerId EventLoop::Schedule(const TimerCallback& cb, Timestamp when) {
  return timer_queue_->AddTimer(cb, when, 0.0);
}

TimerId EventLoop::ScheduleDelay(const TimerCallback& cb, double delay) {
  Timestamp when = AddTime(Timestamp::Now(), delay);
  return Schedule(cb, when);
}

TimerId EventLoop::ScheduleAtFixRate(const TimerCallback& cb, double interval) {
  Timestamp when = AddTime(Timestamp::Now(), interval);
  return timer_queue_->AddTimer(cb, when, interval);
}

void EventLoop::UpdateChannel(Channel* channel) {
  assert(channel->OwnerLoop() == this);
  AssertInLoopThread();
  poller_->UpdateChannel(channel);
}

void EventLoop::AbortNotInLoopThread() {
  LOG_FATAL << "EventLoop::AbortNotInLoopThread - EventLoop " << this
            << " was created in thread_id_ = " << thread_id_
            << ", current thread id = " << current_thread::Tid();
}