// Test13: Multi-threaded TcpServer with 4 IO threads
#include "src/base/logging.h"
#include "src/base/timestamp.h"
#include "src/net/buffer.h"
#include "src/net/event_loop.h"
#include "src/net/tcp_server.h"

#include <atomic>
#include <string>

std::atomic<int> g_connection_count{0};
std::atomic<int> g_message_count{0};

void OnConnection(const muduo::TcpConnectionPtr& conn) {
  if (conn->IsConnected()) {
    g_connection_count++;
    LOG_INFO << "OnConnection(): new connection [" << conn->Name() << "] from "
             << conn->PeerAddress().ToHostPort()
             << " total connections: " << g_connection_count.load();
  } else {
    g_connection_count--;
    LOG_INFO << "OnConnection(): connection [" << conn->Name() << "] is down"
             << " total connections: " << g_connection_count.load();
  }
}

void OnMessage(const muduo::TcpConnectionPtr& conn, muduo::Buffer* buf,
               muduo::Timestamp receive_time) {
  g_message_count++;
  LOG_INFO << conn->Name() << " OnMessage(): " << buf->ReadableBytes() << " bytes, "
           << "total messages: " << g_message_count.load();

  std::string message = buf->ReadAsString();

  // Echo back with thread info
  std::string response = "[Thread " + std::to_string(muduo::current_thread::Tid()) + "] " + message;
  conn->Write(response);
}

int main() {
  muduo::Logger::SetLevel(muduo::Logger::TRACE);
  LOG_INFO << "pid = " << getpid();
  LOG_INFO << "Test13: Multi-threaded TcpServer with 4 IO threads";

  muduo::EventLoop loop;
  muduo::InetAddress listen_addr(9981);
  muduo::TcpServer server(&loop, listen_addr);

  server.SetConnectionCallback(OnConnection);
  server.SetMessageCallback(OnMessage);

  // Set thread count to 4 for multi-threaded mode
  server.SetThreadCount(4);

  server.Start();
  loop.Loop();
  LOG_INFO << "main loop exits";
}
