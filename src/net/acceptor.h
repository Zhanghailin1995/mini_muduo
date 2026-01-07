#ifndef MUDUO_NET_ACCEPTOR_H
#define MUDUO_NET_ACCEPTOR_H

#include "src/base/noncopyable.h"

#include "src/net/channel.h"
#include "src/net/socket.h"

namespace muduo {
class EventLoop;
class InetAddress;

class Acceptor : NonCopyable {
 public:
  typedef std::function<void(int sockfd, const InetAddress&)> NewConnectionCallback;
  Acceptor(EventLoop* loop, const InetAddress& listen_addr);
  // ~Acceptor();

  void Listen();

  bool Listening() const { return listening_; }

  void SetNewConnectionCallback(const NewConnectionCallback& cb) { new_connection_callback_ = cb; }

 private:
  void HandleRead();

  EventLoop* loop_;
  Socket accept_socket_;
  Channel accept_channel_;
  bool listening_;
  NewConnectionCallback new_connection_callback_;
};
}  // namespace muduo

#endif  // MUDUO_NET_ACCEPTOR_H