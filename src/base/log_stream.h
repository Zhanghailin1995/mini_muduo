#ifndef MUDUO_BASE_LOG_STREAM_H
#define MUDUO_BASE_LOG_STREAM_H

#include <cassert>
#include <cstring>
#include <string>

#include "noncopyable.h"

namespace muduo {

namespace detail {
const int K_SMALL_BUFFER = 4000;
const int K_LARGE_BUFFER = 4000 * 1000;

template <int SIZE>
class FixedBuffer {
 public:
  FixedBuffer() : cur_(data_) { SetCookie(CookieStart); }
  ~FixedBuffer() { SetCookie(CookieEnd); }

  void Append(const char *buf, size_t len) {
    if (Available() > static_cast<int>(len)) {
      memcpy(cur_, buf, len);
      cur_ += len;
    }
  }

  const char *Data() const { return data_; }
  int Length() const { return static_cast<int>(cur_ - data_); }
  char *Current() { return cur_; }
  int Available() const { return static_cast<int>(End() - cur_); }
  void Add(size_t len) { cur_ += len; }
  void Reset() { cur_ = data_; }
  void Bzero() { ::bzero(data_, sizeof data_); }

  const char *DebugString();

  void SetCookie(void (*cookie)()) { cookie_ = cookie; }

  std::string ToString() const { return std::string(data_, Length()); }

 private:
  const char *End() const { return data_ + sizeof(data_); }

  // Must be outline function for cookies.
  static void CookieStart();
  static void CookieEnd();

  void (*cookie_)();
  char data_[SIZE];
  char *cur_;
};
}  // namespace detail

class StringHelper {
 public:
  StringHelper(const char *str, int len) : str_(str), len_(len) {
    // if (strlen(str) != static_cast<size_t>(len_)) {
    //   std::cout << "str: " << std::string(str) << "len: " << len << "real len: " << strlen(str) << std::endl;
    // }
    assert(strlen(str) == static_cast<size_t>(len_));
  }

  const char *str_;
  const int len_;
};

class LogStream : NonCopyable {
 public:
  using Buffer = detail::FixedBuffer<detail::K_SMALL_BUFFER>;

  LogStream &operator<<(short);           // NOLINT
  LogStream &operator<<(unsigned short);  // NOLINT
  LogStream &operator<<(int);
  LogStream &operator<<(unsigned int);
  LogStream &operator<<(long);                // NOLINT
  LogStream &operator<<(unsigned long);       // NOLINT
  LogStream &operator<<(long long);           // NOLINT
  LogStream &operator<<(unsigned long long);  // NOLINT

  LogStream &operator<<(const void *);

  LogStream &operator<<(float v) {
    *this << static_cast<double>(v);
    return *this;
  }
  LogStream &operator<<(double);

  LogStream &operator<<(char v) {
    buffer_.Append(&v, 1);
    return *this;
  }

  LogStream &operator<<(const char *str) {
    if (str != nullptr) {
      buffer_.Append(str, strlen(str));
    } else {
      buffer_.Append("(null)", 6);
    }
    return *this;
  }

  LogStream &operator<<(const StringHelper &v) {
    buffer_.Append(v.str_, static_cast<size_t>(v.len_));
    return *this;
  }

  LogStream &operator<<(const std::string &v) {
    buffer_.Append(v.c_str(), v.size());
    return *this;
  }

  void Append(const char *data, int len) { buffer_.Append(data, static_cast<size_t>(len)); }
  const Buffer &InnerBuffer() const { return buffer_; }
  void ResetBuffer() { buffer_.Reset(); }

 private:
  void StaticCheck();
  template <typename T>
  void FormatInteger(T);

  Buffer buffer_;
  static const int K_MAX_NUMERIC_SIZE = 32;
};

class Fmt {
 public:
  template <typename T>
  Fmt(const char *fmt, T val);

  const char *Data() const { return buf_; }
  int Length() const { return length_; }

 private:
  char buf_[32];
  int length_;
};

inline LogStream &operator<<(LogStream &s, const Fmt &fmt) {
  s.Append(fmt.Data(), fmt.Length());
  return s;
}

}  // namespace muduo

#endif  // MUDUO_BASE_LOG_STREAM_H