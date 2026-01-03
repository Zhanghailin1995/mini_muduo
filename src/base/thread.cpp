#include "thread.h"
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include "count_down_latch.h"
#include "current_thread.h"
#include "logging.h"

#include <cassert>
#include <cstdio>
#include <utility>

namespace muduo {

namespace detail {

void AfterFork() {
  current_thread::t_cached_tid = 0;
  current_thread::t_thread_name = "main";
  current_thread::Tid();
}

class ThreadNameInitializer {
 public:
  ThreadNameInitializer() {
    current_thread::t_thread_name = "main";
    current_thread::Tid();
    pthread_atfork(nullptr, nullptr, &AfterFork);
  }
};

ThreadNameInitializer init;

struct ThreadContext {
  using ThreadFunc = std::function<void()>;
  ThreadFunc func_;
  std::string name_;
  pid_t *tid_;
  muduo::CountDownLatch *latch_;

  ThreadContext(ThreadFunc func, std::string name, pid_t *tid, muduo::CountDownLatch *latch)
      : func_(std::move(func)), name_(std::move(name)), tid_(tid), latch_(latch) {}

  void RunInThread() {
    *tid_ = current_thread::Tid();
    tid_ = nullptr;
    latch_->CountDown();
    latch_ = nullptr;
    current_thread::t_thread_name = name_.empty() ? "tiny-muduo-thread" : name_.c_str();
    ::prctl(PR_SET_NAME, current_thread::t_thread_name);
    try {
      func_();
      current_thread::t_thread_name = "finished";
    } catch (const std::exception &ex) {
      current_thread::t_thread_name = "crashed";
      fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
      fprintf(stderr, "reason: %s\n", ex.what());
      abort();
    } catch (...) {
      current_thread::t_thread_name = "crashed";
      fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
      throw;
    }
  }
};

void *StartThread(void *obj) {
  ThreadContext *context = static_cast<ThreadContext *>(obj);
  context->RunInThread();
  delete context;
  return nullptr;
}

}  // namespace detail

std::atomic<int32_t> Thread::num_created;

Thread::Thread(ThreadFunc func, std::string name)
    : started_(false),
      joined_(false),
      pthread_id_(0),
      tid_(0),
      func_(std::move(func)),
      name_(std::move(name)),
      latch_(1) {
  SetDefaultName();
}

Thread::~Thread() {
  if (started_ && !joined_) {
    pthread_detach(pthread_id_);
  }
}

void Thread::SetDefaultName() {
  int num = ++num_created;
  if (name_.empty()) {
    char buf[32];
    snprintf(buf, sizeof buf, "thread-%d", num);
    name_ = buf;
  }
}

void Thread::Start() {
  assert(!started_);
  started_ = true;
  detail::ThreadContext *context = new detail::ThreadContext(func_, name_, &tid_, &latch_);
  if (pthread_create(&pthread_id_, nullptr, &detail::StartThread, context) != 0) {
    started_ = false;
    delete context;
    LOG_SYSFATAL << "Failed in pthread_create";
  } else {
    latch_.Wait();
    assert(tid_ > 0);
  }
}

void Thread::Join() {
  assert(started_);
  assert(!joined_);
  joined_ = true;
  pthread_join(pthread_id_, nullptr);
}

}  // namespace muduo