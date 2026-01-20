//
// Created by hailin on 11/14/22.
//
#include "src/base/logging.h"
#include "src/net/channel.h"
#include "src/net/event_loop.h"

#include <sys/timerfd.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>

using namespace muduo;  // NOLINT

muduo::EventLoop* g_loop;

void Timeout(muduo::Timestamp /*receive_time*/) {
  printf("Timeout!\n");
  g_loop->Quit();
}

int main() {
  Logger::SetLevel(Logger::TRACE);
  auto level = Logger::GetLevelName();
  LOG_INFO << "main(): pid = " << getpid() << ", tid = " << current_thread::Tid()
           << ", log level = " << level;
  muduo::EventLoop loop;
  g_loop = &loop;

  int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  muduo::Channel channel(&loop, timerfd);
  channel.SetReadCallback(Timeout);
  channel.EnableReading();

  struct itimerspec howlong{};
  ::bzero(&howlong, sizeof howlong);
  howlong.it_value.tv_sec = 5;
  ::timerfd_settime(timerfd, 0, &howlong, nullptr);

  loop.Loop();
  ::close(timerfd);
}