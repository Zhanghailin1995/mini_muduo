//
// Created by hailin on 11/14/22.
//

#ifndef MUDUO_BASE_UTILS_H
#define MUDUO_BASE_UTILS_H

#include <cstdint>
#include <string>
namespace muduo {
namespace utils {
template <typename To, typename From>
inline To implicit_cast(From const& f) {  // NOLINT
  return f;
}
}  // namespace utils
}  // namespace muduo

#endif  // MUDUO_BASE_UTILS_H
