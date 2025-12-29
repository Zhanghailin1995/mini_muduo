#include "condition.h"
#include <errno.h>
#include <cstdint>

bool muduo::Condition::WaitForSeconds(double seconds) {
  struct timespec abstime;
  clock_gettime(CLOCK_REALTIME, &abstime);

  const int64_t nano_seconds_per_second = 1000000000;
  int64_t nanoseconds = static_cast<int64_t>(seconds * nano_seconds_per_second);
  abstime.tv_sec += static_cast<time_t>(nanoseconds / nano_seconds_per_second);
  abstime.tv_nsec += static_cast<long>(nanoseconds % nano_seconds_per_second);
  if (abstime.tv_nsec >= nano_seconds_per_second) {
    ++abstime.tv_sec;
    abstime.tv_nsec -= nano_seconds_per_second;
  }

  MutexLock::UnassignGuard ug(mutex_);
  return ETIMEDOUT == pthread_cond_timedwait(&pcond_, mutex_.GetPthreadMutex(), &abstime);
}
