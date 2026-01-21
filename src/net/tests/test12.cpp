// Test12: Single-threaded mode (thread_count = 0, default behavior)
#include "src/base/logging.h"
#include "src/base/timestamp.h"
#include "src/net/buffer.h"
#include "src/net/event_loop.h"
#include "src/net/tcp_server.h"

#include <string>

int g_connection_count = 0;

void OnConnection(const muduo::TcpConnectionPtr& conn) {
  if (conn->IsConnected()) {
    g_connection_count++;
    LOG_INFO << "OnConnection(): new connection [" << conn->Name() << "] from "
             << conn->PeerAddress().ToHostPort()
             << " total connections: " << g_connection_count;
  } else {
    g_connection_count--;
    LOG_INFO << "OnConnection(): connection [" << conn->Name() << "] is down"
             << " total connections: " << g_connection_count;
  }
}

void OnMessage(const muduo::TcpConnectionPtr& conn, muduo::Buffer* buf,
               muduo::Timestamp receive_time) {
  LOG_INFO << conn->Name() << " OnMessage(): " << buf->ReadableBytes() << " bytes received at "
           << receive_time.ToFormattedString();
  std::string message = buf->ReadAsString();
  LOG_INFO << conn->Name() << " OnMessage(): " << message;

  // Echo back
  conn->Write(message);
}

int main() {
  muduo::Logger::SetLevel(muduo::Logger::TRACE);
  LOG_INFO << "pid = " << getpid();
  LOG_INFO << "Test12: Single-threaded TcpServer";

  muduo::EventLoop loop;
  muduo::InetAddress listen_addr(9981);
  muduo::TcpServer server(&loop, listen_addr);

  server.SetConnectionCallback(OnConnection);
  server.SetMessageCallback(OnMessage);

  // Default: single-threaded mode (0 threads)
  server.SetThreadCount(0);

  server.Start();
  loop.Loop();
  LOG_INFO << "main loop exits";
}
