#include "src/net/tcp_connection.h"
#include "src/base/logging.h"
#include "src/net/channel.h"
#include "src/net/event_loop.h"
#include "src/net/socket.h"
#include "src/net/socket_utils.h"

#include <cerrno>
#include <cstdio>

using namespace muduo;  // NOLINT

TcpConnection::TcpConnection(EventLoop* loop, const std::string& name, int sockfd,
                             const InetAddress& local_addr, const InetAddress& peer_addr)
    : loop_(loop),
      name_(name),
      state_(K_CONNECTING),
      socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)),
      local_addr_(local_addr),
      peer_addr_(peer_addr) {
  LOG_DEBUG << "TcpConnection::TcpConnection [" << name_ << "] at " << this << " fd=" << sockfd;
  channel_->SetReadCallback(std::bind(&TcpConnection::HandleRead, this, std::placeholders::_1));
  channel_->SetWriteCallback(std::bind(&TcpConnection::HandleWrite, this));
  channel_->SetErrorCallback(std::bind(&TcpConnection::HandleError, this));
  channel_->SetCloseCallback(std::bind(&TcpConnection::HandleClose, this));
}

TcpConnection::~TcpConnection() {
  LOG_DEBUG << "TcpConnection::~TcpConnection [" << name_ << "] at " << this
            << " fd=" << channel_->Fd();
}

void TcpConnection::ConnectionEstablished() {
  loop_->AssertInLoopThread();
  SetState(K_CONNECTED);
  channel_->EnableReading();

  if (connection_callback_) {
    connection_callback_(shared_from_this());
  }
}

void TcpConnection::ConnectionDestroyed() {
  loop_->AssertInLoopThread();
  assert(state_ == K_CONNECTED);
  SetState(K_DISCONNECTED);
  channel_->DisableAll();
  if (connection_callback_) {
    connection_callback_(shared_from_this());
  }
  channel_->OwnerLoop()->RemoveChannel(channel_.get());
}

void TcpConnection::HandleRead(Timestamp receive_time) {
  int saved_errno = 0;
  ssize_t n = input_buffer_.ReadFd(channel_->Fd(), &saved_errno);
  if (n > 0) {
    if (message_callback_) {
      message_callback_(shared_from_this(), &input_buffer_, receive_time);
    }
  } else if (n == 0) {
    HandleClose();
  } else {
    errno = saved_errno;
    HandleError();
  }
}

void TcpConnection::HandleWrite() {
  // TODO
}

void TcpConnection::HandleClose() {
  LOG_DEBUG << "TcpConnection::HandleClose() fd=" << channel_->Fd();
  loop_->AssertInLoopThread();
  assert(state_ == K_CONNECTED);
  channel_->DisableAll();

  if (close_callback_) {
    close_callback_(shared_from_this());
  }
}

void TcpConnection::HandleError() {
  int err = socket_utils::GetSocketError(channel_->Fd());
  LOG_ERROR << "TcpConnection::HandleError() - SO_ERROR = " << err << " " << strerror_tl(err);
}