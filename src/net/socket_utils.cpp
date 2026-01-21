//
// Created by hailin on 11/17/22.
//
#include "src/net/socket_utils.h"
#include "src/base/logging.h"
#include "src/base/utils.h"

#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>

using namespace muduo;  // NOLINT

namespace {
using SA = struct sockaddr;
#if VALGRIND
void SetNonBlockingAndCloseOnExec(int sockfd) {
  // non-block
  int flags = ::fcntl(sockfd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  int ret = ::fcntl(sockfd, F_SETFL, flags);
  // FIXME check

  // close-on-exec
  flags = ::fcntl(sockfd, F_GETFD, 0);
  flags |= FD_CLOEXEC;
  ret = ::fcntl(sockfd, F_SETFD, flags);
  // FIXME check

  (void)ret;
}
#endif
const SA* SockaddrCast(const struct sockaddr_in* addr) {
  return static_cast<const SA*>(utils::implicit_cast<const void*>(addr));
}

SA* SockaddrCast(struct sockaddr_in* addr) {
  return static_cast<SA*>(utils::implicit_cast<void*>(addr));
}

}  // namespace

int socket_utils::CreateNonblockingSocket() {
#if VALGRIND
  int sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sockfd < 0) {
    LOG_SYSFATAL << "socket_utils::CreateNonblockingSocket";
  }
  SetNonBlockingAndCloseOnExec(sockfd);
#else
  int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
  if (sockfd < 0) {
    LOG_SYSFATAL << "Failed in socket";
  }
#endif
  return sockfd;
}

void socket_utils::Bind(int sockfd, const struct sockaddr_in& addr) {
  int ret = ::bind(sockfd, SockaddrCast(&addr), sizeof addr);
  if (ret < 0) {
    LOG_SYSFATAL << "socket_utils::Bind";
  }
}

void socket_utils::Listen(int sockfd) {
  int ret = ::listen(sockfd, SOMAXCONN);
  if (ret < 0) {
    LOG_SYSFATAL << "socket_utils::Listen";
  }
}

int socket_utils::Accept(int sockfd, struct sockaddr_in* addr) {
  socklen_t addrlen = sizeof *addr;
#if VALGRIND
  int connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
  SetNonBlockingAndCloseOnExec(connfd);
#else
  int connfd = ::accept4(sockfd, SockaddrCast(addr), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
  if (connfd < 0) {
    int saved_errno = errno;
    LOG_SYSERR << "socket_utils::Accept";
    switch (saved_errno) {
      case EAGAIN:
      case ECONNABORTED:
      case EINTR:
      case EPROTO:  // ???
      case EPERM:
      case EMFILE:  // per-process lmit of open file desctiptor ???
        // expected errors
        errno = saved_errno;
        break;
      case EBADF:
      case EFAULT:
      case EINVAL:
      case ENFILE:
      case ENOBUFS:
      case ENOMEM:
      case ENOTSOCK:
      case EOPNOTSUPP:
        // unexpected errors
        LOG_FATAL << "unexpected error of ::accept " << saved_errno;
        break;
      default:
        LOG_FATAL << "unknown error of ::accept " << saved_errno;
        break;
    }
  }
  return connfd;
}

void socket_utils::Close(int sockfd) {
  if (::close(sockfd) < 0) {
    LOG_SYSERR << "socket_utils::Close";
  }
}

void socket_utils::ShutdownWrite(int sockfd) {
  if (::shutdown(sockfd, SHUT_WR) < 0) {
    LOG_SYSERR << "socket_utils::ShutdownWrite";
  }
}

void socket_utils::ToHostPort(char* buf, size_t size, const struct sockaddr_in& addr) {
  char host[INET_ADDRSTRLEN] = "INVALID";
  ::inet_ntop(AF_INET, &addr.sin_addr, host, sizeof host);
  uint16_t port = socket_utils::NetworkToHost16(addr.sin_port);
  snprintf(buf, size, "%s:%u", host, port);
}

void socket_utils::FromHostPort(const char* ip, uint16_t port, struct sockaddr_in* addr) {
  addr->sin_family = AF_INET;
  addr->sin_port = HostToNetwork16(port);
  if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0) {
    LOG_SYSERR << "socket_utils::FromHostPort";
  }
}

struct sockaddr_in socket_utils::GetLocalAddr(int sockfd) {
  struct sockaddr_in local_addr{};
  // memset(&local_addr, 0, sizeof local_addr);
  bzero(&local_addr, sizeof local_addr);
  socklen_t addrlen = sizeof local_addr;
  if (::getsockname(sockfd, SockaddrCast(&local_addr), &addrlen) < 0) {
    LOG_SYSERR << "socket_utils::GetLocalAddr";
  }
  return local_addr;
}

int socket_utils::GetSocketError(int sockfd) {
  int optval;
  socklen_t optlen = sizeof optval;

  if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
    return errno;
  }
  return optval;
}

namespace {
socket_utils::SocketInit kSocketInit;
}  // namespace