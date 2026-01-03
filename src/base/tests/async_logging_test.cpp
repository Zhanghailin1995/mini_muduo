//
// Created by hailin on 11/10/22.
//
#include "src/base/async_logging.h"
#include "src/base/logging.h"

#include <sys/resource.h>
#include <unistd.h>
#include <chrono>
#include <cstdio>

off_t k_roll_size = 50 * 1000 * 1000;

muduo::AsyncLogging *g_async_log = nullptr;

void AsyncOutput(const char *msg, int len) { g_async_log->Append(msg, len); }

void Bench(bool long_log) {
  muduo::Logger::SetOutput(AsyncOutput);
  std::string empty = " ";
  std::string long_str(3000, 'X');
  long_str += " ";

  int cnt = 0;
  const int k_batch = 100000;
  // muduo::Timestamp start = muduo::Timestamp::Now();
  for (int t = 0; t < 30; ++t) {
    auto now = std::chrono::system_clock::now();
    for (int i = 0; i < k_batch; ++i) {
      LOG_INFO << "Hello 0123456789"
               << " abcdefghijklmnopqrstuvwxyz " << (long_log ? long_str : empty) << cnt;
      ++cnt;
    }
    auto duration = std::chrono::system_clock::now() - now;
    auto millis = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    printf("%d batches in %d microseconds\n", k_batch, static_cast<int>(millis));
  }
}

int main(int argc, char *argv[]) {
  {
    // set max virtual memory to 2GB.
    size_t k_one_gb = 1000 * 1024 * 1024;
    rlimit rl = {2 * k_one_gb, 2 * k_one_gb};
    setrlimit(RLIMIT_AS, &rl);
  }

  printf("pid = %d\n", getpid());

  char name[256] = {0};
  strncpy(name, argv[0], sizeof name - 1);
  muduo::AsyncLogging log(::basename(name), k_roll_size);
  log.Start();
  g_async_log = &log;

  bool long_log = argc > 1;
  Bench(long_log);
  return 0;
}