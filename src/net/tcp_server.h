#ifndef MUDUO_NET_TCP_SERVER_H
#define MUDUO_NET_TCP_SERVER_H

#include "src/base/noncopyable.h"
#include "src/net/callbacks.h"
#include "src/net/tcp_connection.h"

#include <map>
#include <memory>

namespace muduo {
class EventLoop;
class Acceptor;

class TcpServer : NonCopyable {
 public:
  TcpServer(EventLoop* loop, const InetAddress& listen_addr);
  ~TcpServer();

  void Start();

  void SetConnectionCallback(const ConnectionCallback& cb) { connection_callback_ = cb; }

  void SetMessageCallback(const MessageCallback& cb) { message_callback_ = cb; }

 private:
  void NewConnection(int sockfd, const InetAddress& peer_addr);

  void RemoveConnection(const TcpConnectionPtr& conn);

  EventLoop* loop_;
  const std::string name_;
  bool started_;
  std::unique_ptr<Acceptor> acceptor_;
  ConnectionCallback connection_callback_;
  MessageCallback message_callback_;

  int next_conn_id_;
  using ConnectionMap = std::map<std::string, TcpConnectionPtr>;
  ConnectionMap connections_;
};
}  // namespace muduo

#endif  // MUDUO_NET_TCP_SERVER_H