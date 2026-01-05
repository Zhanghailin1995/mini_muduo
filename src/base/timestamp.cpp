//
// Created by hailin on 11/29/22.
//
#include "src/base/timestamp.h"
#include <chrono>
#include <cstdio>
#include <ctime>
#include "src/base/utils.h"
#include "timestamp.h"
#define __STDC_FORMAT_MACROS
#include <cinttypes>
#undef __STDC_FORMAT_MACROS

using namespace muduo;  // NOLINT

Timestamp::Timestamp() : micro_seconds_since_epoch_(0) {}

Timestamp::Timestamp(int64_t micro_seconds_since_epoch)
    : micro_seconds_since_epoch_(micro_seconds_since_epoch) {}

std::string Timestamp::ToString() const {
  char buf[32] = {0};
  int64_t seconds = micro_seconds_since_epoch_ / Timestamp::K_MICRO_SECONDS_PER_SECOND;
  int64_t microseconds = micro_seconds_since_epoch_ % Timestamp::K_MICRO_SECONDS_PER_SECOND;
  snprintf(buf, sizeof buf, "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
  return buf;
}

std::string Timestamp::ToFormattedString() const {
  char buf[64] = {0};
  time_t seconds = static_cast<time_t>(micro_seconds_since_epoch_ / K_MICRO_SECONDS_PER_SECOND);
  int microseconds = static_cast<int>(micro_seconds_since_epoch_ % K_MICRO_SECONDS_PER_SECOND);
  struct tm tm_time{};
  ::gmtime_r(&seconds, &tm_time);
  snprintf(buf, sizeof buf, "%4d-%02d-%02d %02d:%02d:%02d.%06d", tm_time.tm_year + 1900,
           tm_time.tm_mon + 1, tm_time.tm_mday, tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
           microseconds);
  // printf("t_time: %s, len: %d\n", t_time, len);
  return buf;
}

Timestamp Timestamp::Now() {
  auto now = std::chrono::system_clock::now();
  auto duration = now.time_since_epoch();
  return Timestamp(std::chrono::duration_cast<std::chrono::microseconds>(duration).count());
}

Timestamp Timestamp::Invalid() { return Timestamp(); }