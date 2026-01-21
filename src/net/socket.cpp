//
// Created by hailin on 11/17/22.
//
#include "src/net/socket.h"
#include "src/net/inet_address.h"
#include "src/net/socket_utils.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <strings.h>

using namespace muduo;  // NOLINT

Socket::~Socket() { socket_utils::Close(sockfd_); }

void Socket::BindAddress(const InetAddress& local_addr) {
  socket_utils::Bind(sockfd_, local_addr.GetSockAddrInet());
}

void Socket::Listen() { socket_utils::Listen(sockfd_); }

int Socket::Accept(InetAddress* peer_addr) {
  struct sockaddr_in addr{};
  bzero(&addr, sizeof(addr));
  int connfd = socket_utils::Accept(sockfd_, &addr);
  if (connfd >= 0) {
    peer_addr->SetSockAddrInet(addr);
  }
  return connfd;
}

void Socket::SetReuseAddr(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof(optval)));
}

void Socket::ShutdownWrite() { socket_utils::ShutdownWrite(sockfd_); }