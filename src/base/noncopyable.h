#ifndef MUDUO_BASE_NONCOPYABLE_H
#define MUDUO_BASE_NONCOPYABLE_H

namespace muduo {
class NonCopyable {
 public:
  NonCopyable(const NonCopyable &) = delete;
  void operator=(const NonCopyable &) = delete;

 protected:
  NonCopyable() = default;
  ~NonCopyable() = default;
};
}  // namespace muduo

#endif  // MUDUO_BASE_NONCOPYABLE_H