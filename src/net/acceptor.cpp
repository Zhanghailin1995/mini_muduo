#include "src/net/acceptor.h"
#include "src/base/logging.h"
#include "src/net/event_loop.h"
#include "src/net/inet_address.h"
#include "src/net/socket_utils.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

using namespace muduo;  // NOLINT

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listen_addr)
    : loop_(loop),
      accept_socket_(socket_utils::CreateNonblockingSocket()),
      accept_channel_(loop, accept_socket_.Fd()),
      listening_(false) {
  accept_socket_.SetReuseAddr(true);
  accept_socket_.BindAddress(listen_addr);
  accept_channel_.SetReadCallback(std::bind(&Acceptor::HandleRead, this));
}

void Acceptor::Listen() {
  loop_->AssertInLoopThread();
  listening_ = true;
  accept_socket_.Listen();
  // when enable reading , the accept channel will be added to poller.
  accept_channel_.EnableReading();
}

void Acceptor::HandleRead() {
  loop_->AssertInLoopThread();
  InetAddress peer_addr(0);
  int connfd = accept_socket_.Accept(&peer_addr);
  // 有一个改进措施，在拿到connfd后，可以非阻塞的使用poll(2)检查一下，
  // 看看fd是否可以写，正常情况下会返回writable，表面connfd可用。
  // 如果poll(2) 返回error，表面connfd有问题，可以直接close掉，避免后续使用时才发现问题。
  if (connfd >= 0) {
    if (new_connection_callback_) {
      LOG_INFO << "Acceptor::HandleRead - new connection from " << peer_addr.ToHostPort();
      new_connection_callback_(connfd, peer_addr);
    } else {
      socket_utils::Close(connfd);
    }
  } else {
    LOG_SYSERR << "in Acceptor::HandleRead";
    if (errno == EMFILE) {
      LOG_ERROR << "Acceptor::HandleRead - out of file descriptor limit";
    }
  }
}
