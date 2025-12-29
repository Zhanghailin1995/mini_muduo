#ifndef MUDUO_BASE_CURRENT_THREAD_H
#define MUDUO_BASE_CURRENT_THREAD_H

#include <unistd.h>

namespace muduo {
namespace current_thread {
extern __thread int t_cached_tid;
extern __thread char t_tid_string[32];
extern __thread int t_tid_string_length;
extern __thread const char *t_thread_name;
void cache_tid();
inline pid_t Tid() {
  if (__builtin_expect(t_cached_tid == 0, 0)) {
    cache_tid();
  }
  return t_cached_tid;
}
inline const char *Name() { return t_thread_name; }
inline int TidStringLength() { return t_tid_string_length; }
inline const char *TidString() { return t_tid_string; }

}  // namespace current_thread
}  // namespace muduo

#endif  // MUDUO_BASE_CURRENT_THREAD_H
