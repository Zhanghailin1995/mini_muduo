//
// Created by hailin on 11/17/22.
//
#include <unistd.h>
#include "src/base/logging.h"
#include "src/net/event_loop.h"
#include "src/net/event_loop_thread.h"

void RunInThread() {
  LOG_INFO << "RunInThread(): pid = " << getpid() << ", tid = " << muduo::current_thread::Tid();
}

int main() {
  muduo::Logger::SetLevel(muduo::Logger::TRACE);
  auto level = muduo::Logger::GetLevelName();
  LOG_INFO << "main(): pid = " << getpid() << ", tid = " << muduo::current_thread::Tid()
           << ", log level = " << level;
  muduo::EventLoopThread loop_thread;
  muduo::EventLoop* loop = loop_thread.StartLoop();
  loop->Execute(RunInThread);
  sleep(1);
  loop->ScheduleDelay(RunInThread, 2);
  sleep(3);
  loop->Quit();
  LOG_INFO << "end";
}