#ifndef MUDUO_NET_EPOLLER_H
#define MUDUO_NET_EPOLLER_H

#include <sys/epoll.h>
#include <cstdint>
#include <vector>

namespace muduo {
class EPoller {
 public:
  EPoller();
  ~EPoller();
  int64_t Poll(int timeout_ms);

 private:
  //   static const int initial_event_list_size_ = 16;
  //   typedef std::vector<struct epoll_event> EventList;
  //   EventList events_;
  int epoll_fd_;
};
}  // namespace muduo

#endif