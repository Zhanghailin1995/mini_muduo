//
// Created by hailin on 11/17/22.
//

#ifndef MUDUO_NET_INET_ADDRESS_H
#define MUDUO_NET_INET_ADDRESS_H

#include <netinet/in.h>
#include <string>
#include "src/base/copyable.h"

namespace muduo {
class InetAddress : public muduo::Copyable {
 public:
  explicit InetAddress(uint16_t port);
  InetAddress(const std::string& ip, uint16_t port);
  explicit InetAddress(const struct sockaddr_in& addr) : addr_(addr) {}

  std::string ToHostPort() const;

  const struct sockaddr_in& GetSockAddrInet() const { return addr_; }
  void SetSockAddrInet(const struct sockaddr_in& addr) { addr_ = addr; }

 private:
  struct sockaddr_in addr_;
};
}  // namespace muduo

#endif  // MUDUO_NET_INET_ADDRESS_H
