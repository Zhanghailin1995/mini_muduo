//
// Created by hailin on 11/16/22.
//
#include "current_thread.h"
#include <sys/syscall.h>
#include <unistd.h>
#include <cstdio>

namespace muduo {
namespace current_thread {
__thread int t_cached_tid = 0;
__thread char t_tid_string[32];
__thread int t_tid_string_length = 6;
__thread const char *t_thread_name = "unknown";

pid_t GetTid() { return static_cast<pid_t>(::syscall(SYS_gettid)); }

void cache_tid() {
  if (t_cached_tid == 0) {
    t_cached_tid = GetTid();
    t_tid_string_length = snprintf(t_tid_string, sizeof t_tid_string, "%5d ", t_cached_tid);
  }
}
}  // namespace current_thread
}  // namespace muduo