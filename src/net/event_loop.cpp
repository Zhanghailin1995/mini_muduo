#include "src/net/event_loop.h"
#include "src/base/current_thread.h"
#include "src/base/logging.h"
#include "src/net/channel.h"
#include "src/net/epoller.h"
#include "src/net/timer_queue.h"

#include <signal.h>
#include <sys/eventfd.h>

using namespace muduo;  // NOLINT

__thread EventLoop* t_loop_in_this_thread = nullptr;
const int K_POLL_TIME_MS = 10 * 1000;

static int CreateEventfd() {
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) {
    LOG_SYSFATAL << "Failed in eventfd";
  }
  return evtfd;
}

class IgnoreSigPipe {
 public:
  IgnoreSigPipe() { ::signal(SIGPIPE, SIG_IGN); }
};

IgnoreSigPipe initObj;

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      running_pending_functors_(false),
      thread_id_(current_thread::Tid()),
      poller_(std::unique_ptr<EPoller>(new EPoller(this))),
      timer_queue_(std::unique_ptr<TimerQueue>(new TimerQueue(this))),
      wakeup_fd_(CreateEventfd()),
      wakeup_channel_(std::unique_ptr<Channel>(new Channel(this, wakeup_fd_))) {
  LOG_TRACE << "EventLoop created " << this << " in thread " << thread_id_;
  if (t_loop_in_this_thread != nullptr) {
    LOG_FATAL << "Another EventLoop " << t_loop_in_this_thread << " exists in this thread "
              << thread_id_;
  } else {
    t_loop_in_this_thread = this;
  }
  wakeup_channel_->SetReadCallback([this](Timestamp /*receive_time*/) {
    uint64_t one = 1;
    ssize_t n = ::read(wakeup_fd_, &one, sizeof one);
    if (n != sizeof one) {
      LOG_ERROR << "EventLoop::HandleRead() reads " << n << " bytes instead of 8";
    }
  });
  wakeup_channel_->EnableReading();
}

EventLoop::~EventLoop() {
  LOG_TRACE << "EventLoop " << this << " of thread " << thread_id_ << " destructs";
  assert(!looping_);
  ::close(wakeup_fd_);
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
      channel->HandleEvent(poll_return_time_);
    }
    RunPendingFunctors();
  }

  LOG_TRACE << "EventLoop " << this << " stop looping";
  looping_ = false;
}

void EventLoop::Quit() {
  quit_ = true;
  if (!IsInLoopThread()) {
    // wake up IO thread (omitted for brevity)
    // There is a chance that loop() just executes while(!quit_) and exists,
    // then EventLoop destructs, then we are accessing an invalid object.
    // Can be fixed using mutex_ in both places.
    // Yet, this is the EventLoop, I assume it's not a hot code path.
    // Thus, leave it as it is now.
    WakeupExecutor();
  }
}

void EventLoop::Execute(Functor cb) {
  if (IsInLoopThread()) {
    cb();
  } else {
    Submit(std::move(cb));
  }
}

void EventLoop::Submit(Functor cb) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    pending_functors_.push_back(std::move(cb));
  }

  // why we need check running_pending_functors_?
  // because user may call Submit() in their callback,
  // if we don't wakeup here, the new added callback may
  // not be executed until next loop.
  if (!IsInLoopThread() || running_pending_functors_) {
    WakeupExecutor();
  }
}

void EventLoop::WakeupExecutor() {
  uint64_t one = 1;
  ssize_t n = ::write(wakeup_fd_, &one, sizeof one);
  if (n != sizeof one) {
    LOG_ERROR << "EventLoop::WakeupExecutor() writes " << n << " bytes instead of 8";
  }
}

void EventLoop::RunPendingFunctors() {
  std::vector<Functor> functors;
  running_pending_functors_ = true;

  {
    std::lock_guard<std::mutex> lock(mutex_);
    functors.swap(pending_functors_);
  }

  for (const Functor& func : functors) {
    func();
  }
  running_pending_functors_ = false;
}

TimerId EventLoop::Schedule(TimerCallback cb, Timestamp when) {
  return timer_queue_->AddTimer(std::move(cb), when, 0.0);
}

TimerId EventLoop::ScheduleDelay(TimerCallback cb, double delay) {
  Timestamp when = AddTime(Timestamp::Now(), delay);
  return Schedule(std::move(cb), when);
}

TimerId EventLoop::ScheduleAtFixRate(TimerCallback cb, double interval) {
  Timestamp when = AddTime(Timestamp::Now(), interval);
  return timer_queue_->AddTimer(std::move(cb), when, interval);
}

void EventLoop::UpdateChannel(Channel* channel) {
  assert(channel->OwnerLoop() == this);
  AssertInLoopThread();
  poller_->UpdateChannel(channel);
}

void EventLoop::RemoveChannel(Channel* channel) {
  assert(channel->OwnerLoop() == this);
  AssertInLoopThread();
  // Implementation of RemoveChannel can be added here if needed.
  poller_->RemoveChannel(channel);
}

void EventLoop::AbortNotInLoopThread() {
  LOG_FATAL << "EventLoop::AbortNotInLoopThread - EventLoop " << this
            << " was created in thread_id_ = " << thread_id_
            << ", current thread id = " << current_thread::Tid();
}