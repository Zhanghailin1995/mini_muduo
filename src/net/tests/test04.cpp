//
// Created by hailin on 11/14/22.
//
#include <unistd.h>
#include "src/base/logging.h"
#include "src/net/event_loop.h"

int cnt = 0;
muduo::EventLoop* g_loop;

void PrintTid() { LOG_INFO << "pid = " << getpid() << ", tid = " << muduo::current_thread::Tid(); }

void Print(const char* msg) {
  LOG_INFO << msg;
  if (++cnt == 20) {
    g_loop->Quit();
  }
}

int main() {
  muduo::Logger::SetLevel(muduo::Logger::TRACE);
  auto level = muduo::Logger::GetLevelName();
  LOG_INFO << "main(): pid = " << getpid() << ", tid = " << muduo::current_thread::Tid()
           << ", log level = " << level;
  muduo::EventLoop loop;
  g_loop = &loop;

  PrintTid();
  loop.ScheduleDelay(std::bind(Print, "once1"), 1);
  loop.ScheduleDelay(std::bind(Print, "once1.5"), 1.5);
  loop.ScheduleDelay(std::bind(Print, "once2.5"), 2.5);
  loop.ScheduleDelay(std::bind(Print, "once3.5"), 3.5);
  //  loop.ScheduleAtFixRate(std::bind(Print, "every1"), 1);
  //  loop.ScheduleAtFixRate(std::bind(Print, "every2"), 2);

  loop.Loop();
}
