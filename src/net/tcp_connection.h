#ifndef MUDUO_NET_TCP_CONNECTION_H
#define MUDUO_NET_TCP_CONNECTION_H

#include "src/base/noncopyable.h"
#include "src/net/callbacks.h"
#include "src/net/inet_address.h"

namespace muduo {
class EventLoop;
class Socket;
class Channel;

class TcpConnection : NonCopyable, public std::enable_shared_from_this<TcpConnection> {
 public:
  TcpConnection(EventLoop* loop, const std::string& name, int sockfd, const InetAddress& local_addr,
                const InetAddress& peer_addr);
  ~TcpConnection();

  EventLoop* GetLoop() const { return loop_; }
  const std::string& Name() const { return name_; }
  const InetAddress& LocalAddress() const { return local_addr_; }
  const InetAddress& PeerAddress() const { return peer_addr_; }
  bool IsConnected() const { return state_ == K_CONNECTED; }

  void SetConnectionCallback(const ConnectionCallback& cb) { connection_callback_ = cb; }

  void SetMessageCallback(const MessageCallback& cb) { message_callback_ = cb; }

  void SetCloseCallback(const CloseCallback& cb) { close_callback_ = cb; }

  // called when TcpServer accepts a new connection
  void ConnectionEstablished();  // should be called only once

  void ConnectionDestroyed();  // should be called only once

 private:
  enum State {
    K_CONNECTING,
    K_CONNECTED,
    K_DISCONNECTED,
  };
  void SetState(State s) { state_ = s; }
  void HandleRead();
  void HandleWrite();
  void HandleClose();
  void HandleError();
  EventLoop* loop_;
  const std::string name_;
  State state_;
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;
  const InetAddress local_addr_;
  const InetAddress peer_addr_;
  ConnectionCallback connection_callback_;
  MessageCallback message_callback_;
  CloseCallback close_callback_;
};
}  // namespace muduo

#endif  // MUDUO_NET_TCP_CONNECTION_H