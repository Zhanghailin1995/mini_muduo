#include "src/net/epoller.h"
#include "src/base/utils.h"

#include <unistd.h>

using namespace muduo;  // NOLINT

EPoller::EPoller() : epoll_fd_(::epoll_create1(EPOLL_CLOEXEC)) {
  if (epoll_fd_ < 0) {
    // Handle error (omitted for brevity)
  }
}

EPoller::~EPoller() { ::close(epoll_fd_); }

int64_t EPoller::Poll(int timeout_ms) {
  const int max_events = 16;
  struct epoll_event events[max_events];

  int num_events = ::epoll_wait(epoll_fd_, events, max_events, timeout_ms);
  int64_t now = utils::MicroSecondsSinceEpoch();

  // Process events (omitted for brevity)

  return now;
}