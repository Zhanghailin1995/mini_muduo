#ifndef MUDUO_NET_CALLBACKS_H
#define MUDUO_NET_CALLBACKS_H

#include <functional>
#include <memory>
#include "src/base/timestamp.h"

namespace muduo {
class Buffer;
class TcpConnection;
typedef std::function<void()> TimerCallback;
typedef std::shared_ptr<class TcpConnection> TcpConnectionPtr;
typedef std::function<void(const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void(const TcpConnectionPtr&, Buffer* buf, Timestamp receive_time)>
    MessageCallback;
typedef std::function<void(const TcpConnectionPtr&)> CloseCallback;
}  // namespace muduo

#endif  // MUDUO_NET_CALLBACKS_H