#ifndef MUDUO_NET_CALLBACKS_H
#define MUDUO_NET_CALLBACKS_H

#include <functional>
#include "src/base/timestamp.h"

namespace muduo {
typedef std::function<void()> TimerCallback;
}

#endif  // MUDUO_NET_CALLBACKS_H