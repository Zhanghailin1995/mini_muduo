//
// Created by hailin on 11/11/22.
//
#include <unistd.h>
#include <thread>
#include "src/base/logging.h"
#include "src/net/event_loop.h"

muduo::EventLoop *g_loop;

void ThreadFunc() {
  LOG_INFO << "ThreadFunc(): pid = " << getpid() << ", tid = " << muduo::current_thread::Tid();
  g_loop->Loop();
}

int main() {
  muduo::Logger::SetLevel(muduo::Logger::TRACE);
  auto level = muduo::Logger::GetLevelName();
  LOG_INFO << "main(): pid = " << getpid() << ", tid = " << muduo::current_thread::Tid() << ", log level = " << level;
  muduo::EventLoop loop;
  g_loop = &loop;
  std::thread thread(ThreadFunc);
  thread.join();
  // pthread_exit(nullptr);
}