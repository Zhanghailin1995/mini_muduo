#ifndef MUDUO_NET_CHANNEL_H
#define MUDUO_NET_CHANNEL_H

#include <functional>
#include "src/base/noncopyable.h"
#include "src/base/timestamp.h"

namespace muduo {
class EventLoop;

class Channel : NonCopyable {
 public:
  typedef std::function<void()> EventCallback;
  typedef std::function<void(Timestamp)> ReadEventCallback;

  Channel(EventLoop* loop, int fd);
  ~Channel();

  void HandleEvent(Timestamp receive_time);
  void SetReadCallback(const ReadEventCallback& cb) { read_callback_ = cb; }
  void SetWriteCallback(const EventCallback& cb) { write_callback_ = cb; }
  void SetErrorCallback(const EventCallback& cb) { error_callback_ = cb; }
  void SetCloseCallback(const EventCallback& cb) { close_callback_ = cb; }

  int Fd() const { return fd_; }
  int Events() const { return events_; }
  void SetRevents(int revt) { revents_ = revt; }
  bool IsNoneEvent() const { return events_ == K_NONE_EVENT; }

  void EnableReading() {
    events_ |= K_READ_EVENT;
    Update();
  }

  void EnableWriting() {
    events_ |= K_WRITE_EVENT;
    Update();
  }

  void DisableWriting() {
    events_ &= ~K_WRITE_EVENT;
    Update();
  }

  void DisableAll() {
    events_ = K_NONE_EVENT;
    Update();
  }

  bool IsInterestedWriting() { return events_ & K_WRITE_EVENT; }

  int Index() { return index_; }

  void SetIndex(int index) { index_ = index; }

  EventLoop* OwnerLoop() { return loop_; }

 private:
  void Update();
  static const int K_NONE_EVENT;
  static const int K_READ_EVENT;
  static const int K_WRITE_EVENT;

  EventLoop* loop_;
  const int fd_;
  int events_;
  int revents_;
  int index_;

  bool event_handling_;

  ReadEventCallback read_callback_;
  EventCallback write_callback_;
  EventCallback error_callback_;
  EventCallback close_callback_;
};
}  // namespace muduo

#endif