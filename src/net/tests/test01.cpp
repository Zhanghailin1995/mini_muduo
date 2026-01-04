//
// Created by hailin on 11/11/22.
//
#include <unistd.h>
#include <thread>
#include "src/base/current_thread.h"
#include "src/base/logging.h"
#include "src/net/event_loop.h"

using namespace muduo;  // NOLINT

void ThreadFunc() {
  LOG_INFO << "ThreadFunc(): pid = " << getpid() << ", tid = " << current_thread::Tid();
  EventLoop loop;
  loop.Loop();
}

int main() {
  Logger::SetLevel(Logger::TRACE);
  auto level = Logger::GetLevelName();
  LOG_INFO << "main(): pid = " << getpid() << ", tid = " << current_thread::Tid() << ", log level = " << level;
  EventLoop loop;
  std::thread thread(ThreadFunc);
  loop.Loop();
  // thread.Start();
  // loop.Loop();
  pthread_exit(nullptr);
}