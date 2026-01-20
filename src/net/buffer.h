//
// Created by hailin on 11/29/22.
//

#ifndef MUDUO_NET_BUFFER_H
#define MUDUO_NET_BUFFER_H

#include <algorithm>
#include <string>
#include <vector>
#include "src/base/copyable.h"

#include <cassert>
namespace muduo {
class Buffer : public Copyable {
 public:
  static const size_t K_CHEAP_PREPEND = 8;
  static const size_t K_INITIAL_SIZE = 1024;

  Buffer()
      : buffer_(K_CHEAP_PREPEND + K_INITIAL_SIZE),
        read_index_(K_CHEAP_PREPEND),
        write_index_(K_CHEAP_PREPEND) {
    assert(ReadableBytes() == 0);
    assert(WritableBytes() == K_INITIAL_SIZE);
    assert(PrependableBytes() == K_CHEAP_PREPEND);
  }

  //  explicit Buffer(size_t initial_size = K_INITIAL_SIZE)
  //      : buffer_(K_CHEAP_PREPEND + initial_size), read_index_(K_CHEAP_PREPEND),
  //      write_index_(K_CHEAP_PREPEND) {
  //    assert(ReadableBytes() == 0);
  //    assert(WritableBytes() == initial_size);
  //    assert(PrependableBytes() == K_CHEAP_PREPEND);
  //  }

  void Swap(Buffer& rhs) {
    buffer_.swap(rhs.buffer_);
    std::swap(read_index_, rhs.read_index_);
    std::swap(write_index_, rhs.write_index_);
  }

  size_t ReadableBytes() const { return write_index_ - read_index_; }
  size_t WritableBytes() const { return buffer_.size() - write_index_; }
  size_t PrependableBytes() const { return read_index_; }

  const char* Peek() const { return RawBufferBegin() + read_index_; }

  void AdvanceReadIndex(size_t len) {
    if (len < ReadableBytes()) {
      read_index_ += len;
    } else {
      Clear();
    }
  }

  void AdvanceReadIndexUntil(const char* end) {
    assert(Peek() <= end);
    assert(end <= BeginWrite());
    AdvanceReadIndex(static_cast<size_t>(end - Peek()));
  }

  void Clear() {
    read_index_ = K_CHEAP_PREPEND;
    write_index_ = K_CHEAP_PREPEND;
  }

  std::string ReadAsString() {
    std::string result(Peek(), ReadableBytes());
    Clear();
    return result;
  }

  void Append(const std::string& str) { Append(str.data(), str.length()); }

  void Append(const char* data, size_t len) {
    EnsureWritableBytes(len);
    std::copy(data, data + len, WritePtr());
    AdvanceWriteIndex(len);
  }

  void EnsureWritableBytes(size_t len) {
    if (WritableBytes() < len) {
      MakeSpace(len);
    }
    assert(WritableBytes() >= len);
  }
  char* BeginWrite() { return RawBufferBegin() + write_index_; }

  const char* BeginWrite() const { return RawBufferBegin() + write_index_; }

  char* WritePtr() { return RawBufferBegin() + write_index_; }

  const char* WritePtr() const { return RawBufferBegin() + write_index_; }

  void AdvanceWriteIndex(size_t len) {
    assert(len <= WritableBytes());
    write_index_ += len;
  }

  void Prepend(const void* data, size_t len) {
    assert(len <= PrependableBytes());
    read_index_ -= len;
    const char* d = static_cast<const char*>(data);
    std::copy(d, d + len, RawBufferBegin() + read_index_);
  }

  void Shrink(size_t reserve) {
    std::vector<char> buf(K_CHEAP_PREPEND + ReadableBytes() + reserve);
    std::copy(Peek(), Peek() + ReadableBytes(), buf.begin() + K_CHEAP_PREPEND);
    buf.swap(buffer_);
  }

  /// Read data directly into buffer.
  /// It may implement with readv(2)
  /// @return result of read(2), @c errno is saved
  ssize_t ReadFd(int fd, int* saved_errno);

 private:
  char* RawBufferBegin() { return &*buffer_.begin(); }
  const char* RawBufferBegin() const { return &*buffer_.begin(); }

  void MakeSpace(size_t len) {
    if (WritableBytes() + PrependableBytes() < len + K_CHEAP_PREPEND) {
      buffer_.resize(write_index_ + len);
    } else {
      assert(K_CHEAP_PREPEND < read_index_);
      size_t readable = ReadableBytes();
      std::copy(RawBufferBegin() + read_index_, RawBufferBegin() + write_index_,
                RawBufferBegin() + K_CHEAP_PREPEND);
      read_index_ = K_CHEAP_PREPEND;
      write_index_ = read_index_ + readable;
      assert(readable == ReadableBytes());
    }
  }

  std::vector<char> buffer_;
  size_t read_index_;
  size_t write_index_;
};
}  // namespace muduo

#endif  // TINY_MUDUO_BUFFER_H
