#include "src/net/event_loop.h"
#include "src/base/current_thread.h"
#include "src/base/logging.h"
#include "src/net/epoller.h"

using namespace muduo;  // NOLINT

__thread EventLoop *t_loop_in_this_thread = nullptr;
const int K_POLL_TIME_MS = 5 * 1000;

EventLoop::EventLoop() : looping_(false), thread_id_(current_thread::Tid()), poller_(std::make_unique<EPoller>()) {
  LOG_TRACE << "EventLoop created " << this << " in thread " << thread_id_;
  if (t_loop_in_this_thread != nullptr) {
    LOG_FATAL << "Another EventLoop " << t_loop_in_this_thread << " exists in this thread " << thread_id_;
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
  LOG_TRACE << "EventLoop " << this << " start looping";
  poller_->Poll(K_POLL_TIME_MS);
  LOG_TRACE << "EventLoop " << this << " stop looping";
  looping_ = false;
}

void EventLoop::AbortNotInLoopThread() {
  LOG_FATAL << "EventLoop::AbortNotInLoopThread - EventLoop " << this << " was created in thread_id_ = " << thread_id_
            << ", current thread id = " << current_thread::Tid();
}