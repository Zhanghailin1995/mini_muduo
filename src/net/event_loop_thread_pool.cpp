#include "src/net/event_loop_thread_pool.h"
#include "src/base/logging.h"
#include "src/net/event_loop.h"
#include "src/net/event_loop_thread.h"

using namespace muduo;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* base_loop)
    : base_loop_(base_loop), num_threads_(0), next_loop_index_(0), started_(false) {}

EventLoopThreadPool::~EventLoopThreadPool() = default;

void EventLoopThreadPool::Start() {
  base_loop_->AssertInLoopThread();
  LOG_DEBUG << "EventLoopThreadPool::Start() - num_threads = " << num_threads_;

  if (started_) {
    LOG_WARN << "EventLoopThreadPool::Start() - already started";
    return;
  }

  started_ = true;

  for (int i = 0; i < num_threads_; ++i) {
    std::unique_ptr<EventLoopThread> thread(new EventLoopThread());
    EventLoop* loop = thread->StartLoop();
    loops_.push_back(loop);
    threads_.push_back(std::move(thread));
  }
}

EventLoop* EventLoopThreadPool::GetNextLoop() {
  base_loop_->AssertInLoopThread();
  EventLoop* loop = base_loop_;

  if (!loops_.empty()) {
    loop = loops_[next_loop_index_];
    ++next_loop_index_;
    if (static_cast<size_t>(next_loop_index_) >= loops_.size()) {
      next_loop_index_ = 0;
    }
  }

  return loop;
}
