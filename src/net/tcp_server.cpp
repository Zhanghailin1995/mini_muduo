#include "src/net/tcp_server.h"
#include "src/base/logging.h"
#include "src/net/acceptor.h"
#include "src/net/event_loop.h"
#include "src/net/socket_utils.h"

#include <cstdio>

using namespace muduo;  // NOLINT

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listen_addr)
    : loop_(loop),
      name_(listen_addr.ToHostPort()),
      started_(false),
      acceptor_(new Acceptor(loop, listen_addr)),
      next_conn_id_(1) {
  acceptor_->SetNewConnectionCallback(
      std::bind(&TcpServer::NewConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() { LOG_DEBUG << "TcpServer::~TcpServer [" << name_ << "] destructs"; }

void TcpServer::Start() {
  if (!started_) {
    started_ = true;
  }
  if (!acceptor_->Listening()) {
    loop_->Execute(std::bind(&Acceptor::Listen, acceptor_.get()));
  }
}

void TcpServer::NewConnection(int sockfd, const InetAddress& peer_addr) {
  loop_->AssertInLoopThread();
  char buf[32];
  snprintf(buf, sizeof(buf), "#%d", next_conn_id_);
  ++next_conn_id_;
  std::string conn_name = name_ + buf;

  LOG_INFO << "TcpServer::NewConnection [" << name_ << "] - new connection " << conn_name
           << " from " << peer_addr.ToHostPort();

  InetAddress local_addr(socket_utils::GetLocalAddr(sockfd));
  TcpConnectionPtr conn(new TcpConnection(loop_, conn_name, sockfd, local_addr, peer_addr));
  connections_[conn_name] = conn;
  // 这个connection_callback_是在TcpServer外部设置的，用于处理新连接建立和断开事件
  conn->SetConnectionCallback(connection_callback_);
  // 这个message_callback_也是在TcpServer外部设置的，用于处理收到消息事件
  conn->SetMessageCallback(message_callback_);
  // 这个write_complete_callback_也是在TcpServer外部设置的，用于处理写完成事件
  conn->SetWriteCompleteCallback(write_complete_callback_);
  conn->SetCloseCallback(std::bind(&TcpServer::RemoveConnection, this, std::placeholders::_1));
  conn->ConnectionEstablished();
}

void TcpServer::RemoveConnection(const TcpConnectionPtr& conn) {
  loop_->AssertInLoopThread();
  LOG_INFO << "TcpServer::RemoveConnection [" << name_ << "] - connection " << conn->Name();
  size_t n = connections_.erase(conn->Name());
  assert(n == 1);
  (void)n;
  // why not call conn->ConnectionDestroyed() directly?
  // because we are in loop thread now,
  // we should let TcpConnection handle the destroy process in its own loop thread.

  loop_->Submit(std::bind(&TcpConnection::ConnectionDestroyed, conn));
}