//
// Created by hailin on 11/17/22.
//
#include "src/net/inet_address.h"
#include <netinet/in.h>
#include <strings.h>
#include "src/net/socket_utils.h"

using namespace muduo;  // NOLINT

static const in_addr_t K_INADDR_ANY = INADDR_ANY;

InetAddress::InetAddress(uint16_t port) {
  bzero(&addr_, sizeof(addr_));
  addr_.sin_family = AF_INET;
  addr_.sin_addr.s_addr = socket_utils::HostToNetwork32(K_INADDR_ANY);
  addr_.sin_port = socket_utils::HostToNetwork16(port);
}

InetAddress::InetAddress(const std::string& ip, uint16_t port) {
  bzero(&addr_, sizeof(addr_));
  socket_utils::FromHostPort(ip.c_str(), port, &addr_);
}

std::string InetAddress::ToHostPort() const {
  char buf[32] = {0};
  socket_utils::ToHostPort(buf, sizeof(buf), addr_);
  return buf;
}
