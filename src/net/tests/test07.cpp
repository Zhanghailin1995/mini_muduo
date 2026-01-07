//
// Created by hailin on 11/17/22.
//
#include <unistd.h>
#include "src/base/logging.h"
#include "src/net/acceptor.h"
#include "src/net/event_loop.h"
#include "src/net/inet_address.h"
#include "src/net/socket_utils.h"

void NewConn(int sockfd, const muduo::InetAddress& peer_addr) {
  LOG_INFO << "NewConn: accepted a new connection from " << peer_addr.ToHostPort();
  ::write(sockfd, "How are you?\n", 13);
  muduo::socket_utils::Close(sockfd);
}

int main() {
  LOG_INFO << "pid = " << getpid();

  muduo::EventLoop loop;

  muduo::InetAddress listen_addr1(9981);
  muduo::Acceptor acceptor(&loop, listen_addr1);
  acceptor.SetNewConnectionCallback(NewConn);
  acceptor.Listen();

  muduo::InetAddress listen_addr2(9982);
  muduo::Acceptor acceptor2(&loop, listen_addr2);
  acceptor2.SetNewConnectionCallback(NewConn);
  acceptor2.Listen();

  loop.Loop();
}