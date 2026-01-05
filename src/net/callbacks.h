#ifndef MINI_MUDUO_CALLBACKS_H
#define MINI_MUDUO_CALLBACKS_H

#include <functional>
#include "src/base/timestamp.h"

namespace muduo {
typedef std::function<void()> TimerCallback;
}

#endif  // MINI_MUDUO_CALLBACKS_H