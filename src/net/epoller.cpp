#include "src/net/epoller.h"
#include "src/base/logging.h"
#include "src/base/utils.h"
#include "src/net/channel.h"

#include <sys/epoll.h>
#include <unistd.h>

using namespace muduo;  // NOLINT

namespace {
const int NEW = -1;
const int ADDED = 1;
const int DELETED = 2;
}  // namespace

EPoller::EPoller(EventLoop* loop)
    : epoll_fd_(::epoll_create1(EPOLL_CLOEXEC)),
      owner_loop_(loop),
      events_(initial_event_list_size_) {
  if (epoll_fd_ < 0) {
    // Handle error (omitted for brevity)
    LOG_SYSFATAL << "EPoller::EPoller() epoll_create1() failed";
  }
}

EPoller::~EPoller() { ::close(epoll_fd_); }

Timestamp EPoller::Poll(int timeout_ms, std::vector<Channel*>* active_channels) {
  epoll_event* evts = &*events_.begin();
  int max_evts = static_cast<int>(events_.size());

  int num_events = ::epoll_wait(epoll_fd_, evts, max_evts, timeout_ms);
  Timestamp now(Timestamp::Now());
  if (num_events > 0) {
    LOG_TRACE << num_events << " events happened";
    FillActiveChannels(num_events, active_channels);
    if (utils::implicit_cast<size_t>(num_events) == events_.size()) {
      events_.resize(events_.size() * 2);
    }
  } else if (num_events == 0) {
    LOG_TRACE << "nothing happened";
  } else {
    // LOG_SYSERR << "EPoller::Poll() error";
    abort();
  }

  // Process events (omitted for brevity)

  return Timestamp(now);
}

void EPoller::FillActiveChannels(int num_events, std::vector<Channel*>* active_channels) const {
  for (int i = 0; i < num_events; ++i) {
    epoll_event event = events_[i];
    Channel* channel = static_cast<Channel*>(event.data.ptr);
#ifndef NDEBUG
    int fd = channel->Fd();
    std::map<int, Channel*>::const_iterator it = channel_map_.find(fd);
    assert(it != channel_map_.end());
    assert(it->second == channel);
#endif
    channel->SetRevents(event.events);
    active_channels->push_back(channel);
  }
}

void EPoller::UpdateChannel(Channel* channel) {
  AssertInLoopThread();
  int fd = channel->Fd();
  int index = channel->Index();
  LOG_TRACE << "EPoller::UpdateChannel fd=" << fd << " events=" << channel->Events()
            << " index=" << index;

  if (index == NEW || index == DELETED) {
    LOG_TRACE << "EPoller::UpdateChannel New channel fd=" << fd;
    if (index == NEW) {
      assert(channel_map_.find(fd) == channel_map_.end());
      channel_map_[fd] = channel;
    } else {
      assert(channel_map_.find(fd) != channel_map_.end());
      assert(channel_map_[fd] == channel);
    }
    channel->SetIndex(ADDED);
    Update(EPOLL_CTL_ADD, channel);
  } else {
    // update existing one
    assert(channel_map_.find(fd) != channel_map_.end());
    assert(channel_map_[fd] == channel);
    if (channel->IsNoneEvent()) {
      Update(EPOLL_CTL_DEL, channel);
      channel->SetIndex(DELETED);
    } else {
      Update(EPOLL_CTL_MOD, channel);
    }
  }
}

void EPoller::RemoveChannel(Channel* channel) {
  AssertInLoopThread();
  int fd = channel->Fd();
  LOG_TRACE << "EPoller::RemoveChannel fd=" << fd;

  assert(channel_map_.find(fd) != channel_map_.end());
  assert(channel_map_[fd] == channel);
  assert(channel->IsNoneEvent());

  int index = channel->Index();
  assert(index == ADDED || index == DELETED);
  size_t n = channel_map_.erase(fd);
  (void)n;
  assert(n == 1);

  if (index == ADDED) {
    Update(EPOLL_CTL_DEL, channel);
  }
  channel->SetIndex(NEW);
}

void EPoller::Update(int operation, Channel* channel) {
  epoll_event event;
  bzero(&event, sizeof event);
  event.events = static_cast<uint32_t>(channel->Events());
  event.data.ptr = channel;
  int fd = channel->Fd();
  if (::epoll_ctl(epoll_fd_, operation, fd, &event) < 0) {
    if (operation == EPOLL_CTL_DEL) {
      LOG_SYSERR << "EPoller::Update() epoll_ctl op = EPOLL_CTL_DEL failed for fd=" << fd;
    } else {
      LOG_SYSFATAL << "EPoller::Update() epoll_ctl op = " << EpollOpToString(operation)
                   << " failed for fd = " << fd << " " << "events = " << channel->Events();
    }
  }
}