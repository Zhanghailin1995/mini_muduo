//
// Created by hailin on 11/19/22.
//
#include "src/base/logging.h"
#include "src/base/timestamp.h"
#include "src/net/buffer.h"
#include "src/net/event_loop.h"
#include "src/net/socket_utils.h"
#include "src/net/tcp_server.h"

void OnConnection(const muduo::TcpConnectionPtr& conn) {
  if (conn->IsConnected()) {
    LOG_INFO << "OnConnection(): new connection [" << conn->Name() << "] from "
             << conn->PeerAddress().ToHostPort();
    conn->Write("你好");
    conn->Write("muduo");
    conn->Shutdown();
  } else {
    LOG_INFO << "OnConnection(): connection [" << conn->Name() << "] is down";
  }
}

void OnMessage(const muduo::TcpConnectionPtr& conn, muduo::Buffer* buf,
               muduo::Timestamp receive_time) {
  LOG_INFO << conn->Name() << " OnMessage(): " << buf->ReadableBytes() << " bytes received at "
           << receive_time.ToFormattedString();
  std::string message = buf->ReadAsString();
  LOG_INFO << conn->Name() << " OnMessage(): " << message;

  // conn->Write(message);
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
  LOG_INFO << "main loop exits";
}