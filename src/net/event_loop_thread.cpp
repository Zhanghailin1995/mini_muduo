//
// Created by hailin on 11/17/22.
//
#include <cassert>
#include "src/net/event_loop.h"
#include "src/net/event_loop_thread.h"

using namespace muduo;  // NOLINT

EventLoopThread::EventLoopThread()
    : loop_(nullptr), thread_(std::bind(&EventLoopThread::ThreadFunc, this)), exiting_(false) {}

EventLoopThread::~EventLoopThread() {
  exiting_ = true;
  loop_->Quit();
  thread_.Join();
}

EventLoop* EventLoopThread::StartLoop() {
  assert(!thread_.Started());
  thread_.Start();
  {
    std::unique_lock<std::mutex> lock(mutex_);
    while (loop_ == nullptr) {
      cond_.wait(lock);
    }
  }
  return loop_;
}

void EventLoopThread::ThreadFunc() {
  EventLoop loop;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    loop_ = &loop;
    cond_.notify_one();
  }
  loop.Loop();
  // assert(exiting_);
  // loop_ = nullptr;
}