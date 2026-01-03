#include "log_stream.h"

#include <algorithm>
#include <limits>

namespace muduo {
namespace detail {
const char DIGITS[] = "9876543210123456789";
const char *zero = DIGITS + 9;
static_assert(sizeof(DIGITS) == 20, "DIGITS length is 20");

const char DIGITS_HEX[] = "0123456789ABCDEF";
static_assert(sizeof(DIGITS_HEX) == 17, "DIGITS_HEX length is 17");

// Efficient Integer to String Conversions, by Matthew Wilson.
template <typename T>
size_t Convert(char buf[], T value) {
  T i = value;
  char *p = buf;

  do {
    int lsd = static_cast<int>(i % 10);
    i /= 10;
    *p++ = zero[lsd];
  } while (i != 0);

  if (value < 0) {
    *p++ = '-';
  }
  *p = '\0';
  std::reverse(buf, p);

  return p - buf;
}

size_t ConvertHex(char buf[], uintptr_t value) {
  uintptr_t i = value;
  char *p = buf;

  do {
    int lsd = static_cast<int>(i % 16);
    i /= 16;
    *p++ = DIGITS_HEX[lsd];
  } while (i != 0);

  *p = '\0';
  std::reverse(buf, p);

  return p - buf;
}
}  // namespace detail
}  // namespace muduo

using namespace muduo;          // NOLINT
using namespace muduo::detail;  // NOLINT

template <int SIZE>
const char *FixedBuffer<SIZE>::DebugString() {
  *cur_ = '\0';
  return data_;
}

template <int SIZE>
void FixedBuffer<SIZE>::CookieStart() {}

template <int SIZE>
void FixedBuffer<SIZE>::CookieEnd() {}

template class muduo::detail::FixedBuffer<K_SMALL_BUFFER>;
template class muduo::detail::FixedBuffer<K_LARGE_BUFFER>;

void LogStream::StaticCheck() {
  static_assert(K_MAX_NUMERIC_SIZE - 10 > std::numeric_limits<double>::digits10, "K_MAX_NUMERIC_SIZE is large enough");
  static_assert(K_MAX_NUMERIC_SIZE - 10 > std::numeric_limits<long double>::digits10,
                "K_MAX_NUMERIC_SIZE is large enough");
  static_assert(K_MAX_NUMERIC_SIZE - 10 > std::numeric_limits<long>::digits10,  // NOLINT
                "K_MAX_NUMERIC_SIZE is large enough");
  static_assert(K_MAX_NUMERIC_SIZE - 10 > std::numeric_limits<long long>::digits10,  // NOLINT
                "K_MAX_NUMERIC_SIZE is large enough");
}

template <typename T>
void LogStream::FormatInteger(T v) {
  if (buffer_.Available() >= K_MAX_NUMERIC_SIZE) {
    size_t len = Convert(buffer_.Current(), v);
    buffer_.Add(len);
  }
}

LogStream &LogStream::operator<<(short v) {  // NOLINT
  *this << static_cast<int>(v);
  return *this;
}

LogStream &LogStream::operator<<(unsigned short v) {  // NOLINT
  *this << static_cast<unsigned int>(v);
  return *this;
}

LogStream &LogStream::operator<<(int v) {
  FormatInteger(v);
  return *this;
}

LogStream &LogStream::operator<<(unsigned int v) {
  FormatInteger(v);
  return *this;
}

LogStream &LogStream::operator<<(long v) {  // NOLINT
  FormatInteger(v);
  return *this;
}

LogStream &LogStream::operator<<(unsigned long v) {  // NOLINT
  FormatInteger(v);
  return *this;
}

LogStream &LogStream::operator<<(long long v) {  // NOLINT
  FormatInteger(v);
  return *this;
}

LogStream &LogStream::operator<<(unsigned long long v) {  // NOLINT
  FormatInteger(v);
  return *this;
}

LogStream &LogStream::operator<<(const void *p) {
  uintptr_t v = reinterpret_cast<uintptr_t>(p);
  if (buffer_.Available() >= K_MAX_NUMERIC_SIZE) {
    char *buf = buffer_.Current();
    buf[0] = '0';
    buf[1] = 'x';
    size_t len = ConvertHex(buf + 2, v);
    buffer_.Add(len + 2);
  }
  return *this;
}

LogStream &LogStream::operator<<(double v) {
  if (buffer_.Available() >= K_MAX_NUMERIC_SIZE) {
    int len = snprintf(buffer_.Current(), K_MAX_NUMERIC_SIZE, "%.12g", v);
    buffer_.Add(static_cast<size_t>(len));
  }
  return *this;
}

template <typename T>
Fmt::Fmt(const char *fmt, T val) {
  static_assert(std::is_arithmetic<T>::value == true, "Must be arithmetic type");
  length_ = static_cast<int>(snprintf(buf_, sizeof buf_, fmt, val));
  assert(length_ < static_cast<int>(sizeof buf_));
}

// Explicit instantiations
template Fmt::Fmt(const char *fmt, char);
template Fmt::Fmt(const char *fmt, signed char);
template Fmt::Fmt(const char *fmt, unsigned char);
template Fmt::Fmt(const char *fmt, short);
template Fmt::Fmt(const char *fmt, unsigned short);
template Fmt::Fmt(const char *fmt, int);
template Fmt::Fmt(const char *fmt, unsigned int);
template Fmt::Fmt(const char *fmt, long);
template Fmt::Fmt(const char *fmt, unsigned long);
template Fmt::Fmt(const char *fmt, long long);
template Fmt::Fmt(const char *fmt, unsigned long long);
template Fmt::Fmt(const char *fmt, float);
template Fmt::Fmt(const char *fmt, double);
