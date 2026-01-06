//
// Created by hailin on 11/16/22.
//
#include <unistd.h>
#include <cstdio>
#include "src/base/logging.h"
#include "src/net/event_loop.h"

muduo::EventLoop* g_loop;

int g_flag = 0;

void Run4() {
  LOG_INFO << "Run4(): pid = " << getpid() << ", tid = " << muduo::current_thread::Tid()
           << ", flag = " << g_flag;
  g_loop->Quit();
}

void Run3() {
  LOG_INFO << "Run3(): pid = " << getpid() << ", tid = " << muduo::current_thread::Tid()
           << ", flag = " << g_flag;
  g_loop->ScheduleDelay(Run4, 1);
  g_flag = 3;
}

void Run2() {
  LOG_INFO << "Run2(): pid = " << getpid() << ", tid = " << muduo::current_thread::Tid()
           << ", flag = " << g_flag;
  g_loop->Submit(Run3);
}

void Run1() {
  LOG_INFO << "Run1(): pid = " << getpid() << ", tid = " << muduo::current_thread::Tid()
           << ", flag = " << g_flag;
  g_loop->Execute(Run2);
  g_flag = 2;
}

int main() {
  muduo::Logger::SetLevel(muduo::Logger::TRACE);
  auto level = muduo::Logger::GetLevelName();
  LOG_INFO << "main(): pid = " << getpid() << ", tid = " << muduo::current_thread::Tid()
           << ", log level = " << level;
  muduo::EventLoop loop;
  g_loop = &loop;

  g_loop->ScheduleDelay(Run1, 2);

  g_loop->Loop();
  LOG_INFO << "end, flag = " << g_flag;
}