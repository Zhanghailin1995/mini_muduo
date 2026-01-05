//
// Created by hailin on 11/14/22.
//

#ifndef MINI_MUDUO_UTILS_H
#define MINI_MUDUO_UTILS_H

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

#endif  // MINI_MUDUO_UTILS_H
