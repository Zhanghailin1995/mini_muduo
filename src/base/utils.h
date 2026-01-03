//
// Created by hailin on 11/14/22.
//

#ifndef TINY_MUDUO_UTILS_H
#define TINY_MUDUO_UTILS_H

#include <cstdint>
#include <string>
namespace muduo {
namespace utils {
static const int K_MICRO_SECONDS_PER_SECOND = 1000 * 1000;
int64_t MicroSecondsSinceEpoch();
std::string FormatTime(int64_t micro_seconds_since_epoch);
template <typename To, typename From>
inline To implicit_cast(From const &f) {  // NOLINT
  return f;
}
}  // namespace utils
}  // namespace muduo

#endif  // TINY_MUDUO_UTILS_H
