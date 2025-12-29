#include "src/base/mutex.h"
#include "src/base/noncopyable.h"

class Counter : muduo ::NonCopyable {
 public:
  Counter() : value_(0) {}
  void GetAndIncrement() {
    muduo::MutexLockGuard lock(mutex_);
    ++value_;
  }
  int value() const {
    muduo::MutexLockGuard lock(mutex_);
    return value_;
  }

 private:
  int value_;
  mutable muduo::MutexLock mutex_;
};

#include <iostream>
#include <thread>
#include <vector>

int main() {
  Counter counter;
  const int numThreads = 4;
  const int incrementsPerThread = 1000;

  std::vector<std::thread> threads;
  for (int i = 0; i < numThreads; ++i) {
    threads.emplace_back([&counter, incrementsPerThread]() {
      for (int j = 0; j < incrementsPerThread; ++j) {
        counter.GetAndIncrement();
      }
    });
  }

  for (auto &t : threads) {
    t.join();
  }

  std::cout << "Final counter value: " << counter.value() << std::endl;
  std::cout << "Expected value: " << (numThreads * incrementsPerThread) << std::endl;

  return 0;
}
