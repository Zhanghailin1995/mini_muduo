#ifndef MUDUO_NET_CALLBACKS_H
#define MUDUO_NET_CALLBACKS_H

#include <functional>
#include <memory>
#include "src/base/timestamp.h"

namespace muduo {
class TcpConnection;
typedef std::function<void()> TimerCallback;
typedef std::shared_ptr<class TcpConnection> TcpConnectionPtr;
typedef std::function<void(const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void(const TcpConnectionPtr&, const char* data, ssize_t len)> MessageCallback;
typedef std::function<void(const TcpConnectionPtr&)> CloseCallback;
}  // namespace muduo

#endif  // MUDUO_NET_CALLBACKS_H