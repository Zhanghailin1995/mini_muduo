//
// Created by hailin on 11/14/22.
//
#include "utils.h"

#include <chrono>
#include <string>

using namespace muduo::utils;  // NOLINT

int64_t muduo::utils::MicroSecondsSinceEpoch() {
  auto now = std::chrono::system_clock::now();
  auto duration = now.time_since_epoch();
  return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}

std::string muduo::utils::FormatTime(int64_t micro_seconds_since_epoch) {
  char buf[64] = {0};
  time_t seconds = static_cast<time_t>(micro_seconds_since_epoch / K_MICRO_SECONDS_PER_SECOND);
  int microseconds = static_cast<int>(micro_seconds_since_epoch % K_MICRO_SECONDS_PER_SECOND);
  struct tm tm_time {};
  ::gmtime_r(&seconds, &tm_time);
  snprintf(buf, sizeof buf, "%4d-%02d-%02d %02d:%02d:%02d.%06d", tm_time.tm_year + 1900, tm_time.tm_mon + 1,
           tm_time.tm_mday, tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, microseconds);
  // printf("t_time: %s, len: %d\n", t_time, len);
  return buf;
}