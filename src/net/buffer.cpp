//
// Created by hailin on 11/29/22.
//
#include "src/net/buffer.h"
#include "src/base/logging.h"
#include "src/net/socket_utils.h"

#include <memory.h>
#include <sys/uio.h>
#include <cerrno>

using namespace muduo;  // NOLINT
ssize_t Buffer::ReadFd(int fd, int *saved_errno) {
  char extra_buf[65536];
  struct iovec vec[2];
  const size_t writable = WritableBytes();
  vec[0].iov_base = RawBufferBegin() + write_index_;
  vec[0].iov_len = writable;
  vec[1].iov_base = extra_buf;
  vec[1].iov_len = sizeof(extra_buf);
  // const int iovcnt = (writable < sizeof(extra_buf)) ? 2 : 1;
  const int iovcnt = 2;
  const ssize_t n = readv(fd, vec, iovcnt);
  if (n < 0) {
    *saved_errno = errno;
  } else if (static_cast<size_t>(n) <= writable) {
    write_index_ += static_cast<size_t>(n);
  } else {
    write_index_ = buffer_.size();
    Append(extra_buf, static_cast<size_t>(n) - writable);
  }
  return n;
}
