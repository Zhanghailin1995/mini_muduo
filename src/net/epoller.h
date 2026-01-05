#ifndef MUDUO_NET_EPOLLER_H
#define MUDUO_NET_EPOLLER_H

#include <sys/epoll.h>
#include <map>
#include <vector>
#include "src/base/timestamp.h"
#include "src/net/event_loop.h"

namespace muduo {
inline const char* EpollOpToString(int op) {
  switch (op) {
    case EPOLL_CTL_ADD:
      return "EPOLL_CTL_ADD";
    case EPOLL_CTL_DEL:
      return "EPOLL_CTL_DEL";
    case EPOLL_CTL_MOD:
      return "EPOLL_CTL_MOD";
    default:
      return "EPOLL_CTL_UNKNOWN";
  }
}
}  // namespace muduo

struct epoll_event;

namespace muduo {

class Channel;
class EPoller : NonCopyable {
 public:
  EPoller(EventLoop* loop);
  ~EPoller();
  Timestamp Poll(int timeout_ms, std::vector<Channel*>* active_channels);

  // changes the interested events
  // must be called in the loop thread
  void UpdateChannel(Channel* channel);
  // remove the channel , when it destructs.
  // must be called in the loop thread
  // void RemoveChannel(Channel *channel);

  void AssertInLoopThread() { owner_loop_->AssertInLoopThread(); }

 private:
  static const int initial_event_list_size_ = 16;

  void FillActiveChannels(int num_events, std::vector<Channel*>* active_channels) const;
  void Update(int operation, Channel* channel);

  int epoll_fd_;
  EventLoop* owner_loop_;
  std::vector<struct epoll_event> events_;
  std::map<int, Channel*> channel_map_;
};
}  // namespace muduo

#endif