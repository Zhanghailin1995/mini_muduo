#include "src/net/tcp_server.h"
#include "src/base/logging.h"
#include "src/net/acceptor.h"
#include "src/net/event_loop.h"
#include "src/net/event_loop_thread_pool.h"
#include "src/net/socket_utils.h"

#include <cstdio>

using namespace muduo;  // NOLINT

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listen_addr)
    : loop_(loop),
      name_(listen_addr.ToHostPort()),
      started_(false),
      acceptor_(new Acceptor(loop, listen_addr)),
      thread_pool_(new EventLoopThreadPool(loop)),
      next_conn_id_(1) {
  acceptor_->SetNewConnectionCallback(
      std::bind(&TcpServer::NewConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() { LOG_DEBUG << "TcpServer::~TcpServer [" << name_ << "] destructs"; }

void TcpServer::Start() {
  if (!started_) {
    started_ = true;
    thread_pool_->Start();
  }
  if (!acceptor_->Listening()) {
    loop_->Execute(std::bind(&Acceptor::Listen, acceptor_.get()));
  }
}

void TcpServer::SetThreadCount(int num_threads) { thread_pool_->SetThreadCount(num_threads); }

void TcpServer::NewConnection(int sockfd, const InetAddress& peer_addr) {
  loop_->AssertInLoopThread();
  char buf[32];
  snprintf(buf, sizeof(buf), "#%d", next_conn_id_);
  ++next_conn_id_;
  std::string conn_name = name_ + buf;

  LOG_INFO << "TcpServer::NewConnection [" << name_ << "] - new connection " << conn_name
           << " from " << peer_addr.ToHostPort();

  InetAddress local_addr(socket_utils::GetLocalAddr(sockfd));
  // Get IO loop from thread pool
  EventLoop* io_loop = thread_pool_->GetNextLoop();
  TcpConnectionPtr conn(new TcpConnection(io_loop, conn_name, sockfd, local_addr, peer_addr));
  connections_[conn_name] = conn;
  // 这个connection_callback_是在TcpServer外部设置的，用于处理新连接建立和断开事件
  conn->SetConnectionCallback(connection_callback_);
  // 这个message_callback_也是在TcpServer外部设置的，用于处理收到消息事件
  conn->SetMessageCallback(message_callback_);
  // 这个write_complete_callback_也是在TcpServer外部设置的，用于处理写完成事件
  conn->SetWriteCompleteCallback(write_complete_callback_);
  conn->SetCloseCallback(std::bind(&TcpServer::RemoveConnection, this, std::placeholders::_1));
  // Run ConnectionEstablished in the IO loop thread
  io_loop->Execute([conn]() { conn->ConnectionEstablished(); });
}

void TcpServer::RemoveConnection(const TcpConnectionPtr& conn) {
  // Called from TcpConnection's IO thread
  // Queue to server's loop thread for thread-safe map access
  loop_->Execute(std::bind(&TcpServer::RemoveConnectionInEventExecutor, this, conn));
}

void TcpServer::RemoveConnectionInEventExecutor(const TcpConnectionPtr& conn) {
  loop_->AssertInLoopThread();
  LOG_INFO << "TcpServer::RemoveConnection [" << name_ << "] - connection " << conn->Name();
  size_t n = connections_.erase(conn->Name());
  assert(n == 1);
  (void)n;
  // The connection's IO loop may be different from the accept loop
  // So we need to run ConnectionDestroyed in the connection's own loop thread

  EventLoop* io_loop = conn->GetLoop();
  io_loop->Submit([conn]() { conn->ConnectionDestroyed(); });
}