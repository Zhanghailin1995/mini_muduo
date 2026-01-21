//
// Created by hailin on 11/17/22.
//

#ifndef MUDUO_NET_SOCKET_UTILS_H
#define MUDUO_NET_SOCKET_UTILS_H

#include <arpa/inet.h>
#include <endian.h>

namespace muduo {
namespace socket_utils {
inline uint64_t HostToNetwork64(uint64_t host64) { return htobe64(host64); }

inline uint32_t HostToNetwork32(uint32_t host32) { return htonl(host32); }

inline uint16_t HostToNetwork16(uint16_t host16) { return htons(host16); }

inline uint64_t NetworkToHost64(uint64_t net64) { return be64toh(net64); }

inline uint32_t NetworkToHost32(uint32_t net32) { return ntohl(net32); }

inline uint16_t NetworkToHost16(uint16_t net16) { return ntohs(net16); }

int CreateNonblockingSocket();

void Bind(int sockfd, const struct sockaddr_in& addr);

void Listen(int sockfd);

int Accept(int sockfd, struct sockaddr_in* addr);

void Close(int sockfd);

void ShutdownWrite(int sockfd);

void ToHostPort(char* buf, size_t size, const struct sockaddr_in& addr);

void FromHostPort(const char* ip, uint16_t port, struct sockaddr_in* addr);

struct sockaddr_in GetLocalAddr(int sockfd);

int GetSocketError(int sockfd);

}  // namespace socket_utils
}  // namespace muduo
#endif  // MUDUO_NET_SOCKET_UTILS_H
