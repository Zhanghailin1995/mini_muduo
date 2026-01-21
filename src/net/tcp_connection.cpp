#include "src/net/tcp_connection.h"
#include "src/base/logging.h"
#include "src/base/utils.h"
#include "src/net/channel.h"
#include "src/net/event_loop.h"
#include "src/net/socket.h"
#include "src/net/socket_utils.h"

#include <cerrno>
#include <cstdio>

using namespace muduo;         // NOLINT
using namespace muduo::utils;  // NOLINT

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

void TcpConnection::Write(const std::string& message) {
  Write(message.data(), static_cast<int>(message.size()));
}

void TcpConnection::Write(const void* data, int len) {
  if (loop_->IsInLoopThread()) {
    WriteInEventExecutor(data, len);
  } else {
    std::string message(static_cast<const char*>(data), len);
    loop_->Execute([this, message]() {
      WriteInEventExecutor(message.data(), static_cast<int>(message.size()));
    });
  }
}

void TcpConnection::WriteInEventExecutor(const void* data, int len) {
  loop_->AssertInLoopThread();
  if (state_ == K_DISCONNECTED) {
    LOG_WARN << "TcpConnection::WriteInEventExecutor() - disconnected, give up writing";
    return;
  }
  ssize_t nwrote = 0;
  // if no data in output buffer, try writing directly
  if (!channel_->IsInterestedWriting() && output_buffer_.ReadableBytes() == 0) {
    nwrote = ::write(channel_->Fd(), data, static_cast<size_t>(len));
    if (nwrote >= 0) {
      if (implicit_cast<size_t>(nwrote) < static_cast<size_t>(len)) {
        LOG_TRACE << "TcpConnection::WriteInEventExecutor() - partial write" << nwrote
                  << " bytes written out of " << len;
      } else {
        // all data written in one shot
        if (write_complete_callback_) {
          loop_->Submit([this]() {
            if (write_complete_callback_) {
              write_complete_callback_(shared_from_this());
            }
          });
        }
      }
    } else {
      nwrote = 0;
      if (errno != EWOULDBLOCK) {
        LOG_SYSERR << "TcpConnection::WriteInEventExecutor()";
      }
    }
  }

  assert(nwrote >= 0);
  if (implicit_cast<size_t>(nwrote) < static_cast<size_t>(len)) {
    // append remaining data to output buffer
    output_buffer_.Append(static_cast<const char*>(data) + nwrote,
                          static_cast<size_t>(len - nwrote));
    if (!channel_->IsInterestedWriting()) {
      channel_->EnableWriting();
    }
  }
}

void TcpConnection::Shutdown() {
  if (state_ == K_CONNECTED) {
    SetState(K_DISCONNECTING);
    loop_->Execute(std::bind(&TcpConnection::ShutdownInEventExecutor, this));
  }
}

void TcpConnection::SetTcpNoDelay(bool on) { socket_->SetTcpNoDelay(on); }

void TcpConnection::SetKeepAlive(bool on) { socket_->SetKeepAlive(on); }

void TcpConnection::ShutdownInEventExecutor() {
  loop_->AssertInLoopThread();
  if (!channel_->IsInterestedWriting()) {
    socket_->ShutdownWrite();
  }
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
  assert(state_ == K_CONNECTED || state_ == K_DISCONNECTING);
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
  loop_->AssertInLoopThread();
  if (channel_->IsInterestedWriting()) {
    ssize_t n = ::write(channel_->Fd(), output_buffer_.Peek(),
                        static_cast<size_t>(output_buffer_.ReadableBytes()));
    if (n > 0) {
      output_buffer_.AdvanceReadIndex(static_cast<size_t>(n));
      if (output_buffer_.ReadableBytes() == 0) {
        // all data sent
        channel_->DisableWriting();
        if (write_complete_callback_) {
          loop_->Submit([this]() {
            if (write_complete_callback_) {
              write_complete_callback_(shared_from_this());
            }
          });
        }
        if (state_ == K_DISCONNECTING) {
          ShutdownInEventExecutor();
        }
      }
    } else {
      LOG_SYSERR << "TcpConnection::HandleWrite()";
    }
  } else {
    LOG_TRACE << "TcpConnection::HandleWrite() - not interested in writing";
  }
}

void TcpConnection::HandleClose() {
  LOG_DEBUG << "TcpConnection::HandleClose() fd=" << channel_->Fd();
  loop_->AssertInLoopThread();
  assert(state_ == K_CONNECTED || state_ == K_DISCONNECTING);
  channel_->DisableAll();

  if (close_callback_) {
    close_callback_(shared_from_this());
  }
}

void TcpConnection::HandleError() {
  int err = socket_utils::GetSocketError(channel_->Fd());
  LOG_ERROR << "TcpConnection::HandleError() - SO_ERROR = " << err << " " << strerror_tl(err);
}