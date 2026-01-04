#include "src/net/channel.h"
#include "src/base/logging.h"

#include "src/net/event_loop.h"

#include <poll.h>

using namespace muduo;  // NOLINT

const int Channel::K_NONE_EVENT = 0;
const int Channel::K_READ_EVENT = POLLIN | POLLPRI;
const int Channel::K_WRITE_EVENT = POLLOUT;

Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1) {}

void Channel::Update() { loop_->UpdateChannel(this); }

void Channel::HandleEvent() {
  if (revents_ & POLLNVAL) {
    LOG_WARN << "Channel::HandleEvent() POLLNVAL";
  }

  if (revents_ & (POLLERR | POLLNVAL)) {
    if (error_callback_) error_callback_();
  }
  if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
    if (read_callback_) read_callback_();
  }
  if (revents_ & POLLOUT) {
    if (write_callback_) write_callback_();
  }
}