//
// Created by hailin on 11/19/22.
//
#include "src/base/logging.h"
#include "src/net/event_loop.h"
#include "src/net/tcp_server.h"

void OnConnection(const muduo::TcpConnectionPtr& conn) {
  if (conn->IsConnected()) {
    LOG_INFO << "OnConnection(): new connection [" << conn->Name() << "] from "
             << conn->PeerAddress().ToHostPort();
  } else {
    LOG_INFO << "OnConnection(): connection [" << conn->Name() << "] is down";
  }
}

void OnMessage(const muduo::TcpConnectionPtr& conn, const char* buf, ssize_t len) {
  LOG_INFO << conn->Name() << " OnMessage(): " << len << " bytes received from connection "
           << conn->Name();
  LOG_INFO << conn->Name() << " OnMessage(): " << std::string(buf, len);
}

int main() {
  muduo::Logger::SetLevel(muduo::Logger::TRACE);
  LOG_INFO << "pid = " << getpid();
  muduo::EventLoop loop;
  muduo::InetAddress listen_addr(9981);
  muduo::TcpServer server(&loop, listen_addr);
  server.SetConnectionCallback(OnConnection);
  server.SetMessageCallback(OnMessage);
  server.Start();
  loop.Loop();
}