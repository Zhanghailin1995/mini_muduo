//
// Created by hailin on 11/17/22.
//

#ifndef MUDUO_NET_SOCKET_H
#define MUDUO_NET_SOCKET_H
#include "src/base/noncopyable.h"

namespace muduo {
class InetAddress;

/// Wrapper of socket file descriptor.
///
/// It closes the sockfd when desctructs.
/// It's thread safe, all operations are delegated to OS.

class Socket : NonCopyable {
 public:
  explicit Socket(int sockfd) : sockfd_(sockfd) {}
  ~Socket();

  int Fd() const { return sockfd_; }

  // abort if address in use
  void BindAddress(const InetAddress& local_addr);
  void Listen();

  // On success, returns a non-negative integer that is a descriptor for the accepted socket,
  // which has been set to non-blocking and close-on-exec. *peeraddr is assigned.
  // On error, -1 is returned, and *peeraddr is untouched.
  int Accept(InetAddress* peeraddr);

  // void ShutdownWrite();

  /// Enable/disable TCP_NODELAY (disable/enable Nagle's algorithm).
  /// void SetTcpNoDelay(bool on);

  /// Enable/disable SO_REUSEADDR
  void SetReuseAddr(bool on);

  /// Enable/disable SO_REUSEPORT
  // void SetReusePort(bool on);

  /// Enable/disable SO_KEEPALIVE
  // void SetKeepAlive(bool on);

 private:
  const int sockfd_;
};
}  // namespace muduo
#endif  // MUDUO_NET_SOCKET_H
