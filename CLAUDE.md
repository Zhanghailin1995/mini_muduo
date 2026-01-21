# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**mini_muduo** is a lightweight C++ network library implementing the Reactor pattern with epoll for I/O multiplexing. It follows the "one loop per thread" threading model inspired by the Muduo library.

- **Language:** C++11
- **Platform:** Linux (uses epoll, timerfd, eventfd)
- **Build System:** CMake

## Build Commands

```bash
# Configure and build
mkdir -p build && cd build
cmake ..
make -j$(nproc)

# Build specific test
make test01

# Run tests
./bin/test01    # Basic event loop
./bin/test13    # Multi-threaded TCP server

# Test with netcat (in another terminal)
nc localhost 9981
```

## High-Level Architecture

### Reactor Pattern with One Loop Per Thread

The library is built around the Reactor pattern where each thread runs an `EventLoop`:

```
TcpServer
    │
    ├─ Acceptor (accepts connections in main loop)
    │
    └─ EventLoopThreadPool (distributes connections to worker loops)
            │
            └─ EventLoop per thread (epoll-based I/O multiplexing)
                    │
                    ├─ EPoller (epoll wrapper)
                    ├─ TimerQueue (timerfd-based timers)
                    ├─ Channel (fd + callbacks)
                    └─ pending_functors_ (cross-thread task queue)
```

### Key Design Principles

1. **Thread Affinity:** Each `EventLoop` is bound to a single thread. `AssertInLoopThread()` catches cross-thread misuse.
2. **Cross-thread Dispatch:** Use `Execute(cb)` (immediate or queued) or `Submit(cb)` (always queued) to run functors in an EventLoop's thread.
3. **RAII:** All resources (file descriptors, threads) are managed by destructors.
4. **NonCopyable:** Classes managing resources inherit from `NonCopyable` to prevent copy/assignment.

## Core Components

### EventLoop (`src/net/event_loop.h`)
The heart of the reactor - each thread runs one EventLoop.
- `Loop()` - Start the event loop (blocks until `Quit()`)
- `Quit()` - Stop the event loop
- `Execute(Functor)` - Run in loop thread (immediate if same thread, queued otherwise)
- `Submit(Functor)` - Queue for execution in loop thread
- `Schedule(TimerCallback, when/delay)` - Add timer
- `UpdateChannel(Channel*)` - Register fd with epoll

### Channel (`src/net/channel.h`)
Wraps a file descriptor and callbacks for I/O events.
- Holds callbacks: `read_callback_`, `write_callback_`, `close_callback_`, `error_callback_`
- Owned by `TcpConnection` or `Acceptor`, registered with `EventLoop`

### TcpServer (`src/net/tcp_server.h`)
Multi-threaded TCP server.
- **Main loop:** Uses `Acceptor` to accept new connections
- **Worker loops:** `EventLoopThreadPool` handles I/O for connections
- Connections are distributed round-robin across worker threads
- User sets callbacks: `ConnectionCallback`, `MessageCallback`, `WriteCompleteCallback`

### TcpConnection (`src/net/tcp_connection.h`)
Manages a single TCP connection.
- State machine: `K_CONNECTING → K_CONNECTED → K_DISCONNECTING → K_DISCONNECTED`
- Uses `std::enable_shared_from_this` for safe `shared_ptr` passing to callbacks
- Automatic SIGPIPE handling

### Buffer (`src/net/buffer.h`)
Network buffer with automatic growth/compaction.
- Structure: `[prependable] [readable] [writable]`
- `ReadFd(int fd, int* saved_errno)` - Direct read into buffer using readv(2)
- `Append(const char* data, size_t len)` - Add data to buffer
- `RetrieveAsString()` - Consume readable bytes as string

### EPoller (`src/net/epoller.h`)
Wrapper around Linux epoll.
- `Poll(int timeout_ms, vector<Channel*>* active_channels)` - Wait for events
- `UpdateChannel(Channel*)` - EPOLL_CTL_ADD/MOD
- `RemoveChannel(Channel*)` - EPOLL_CTL_DEL

### Timer Mechanism (`src/net/timer_queue.h`)
Uses `timerfd_create()` for timer integration with epoll.
- Timers stored in `std::set<std::pair<Timestamp, Timer*>>`
- API: `Schedule()`, `ScheduleDelay()`, `ScheduleAtFixRate()`

## Callback System (`src/net/callbacks.h`)

```cpp
ConnectionCallback    void(const TcpConnectionPtr&)          // connect/disconnect
MessageCallback       void(const TcpConnectionPtr&, Buffer*, Timestamp)  // data received
WriteCompleteCallback void(const TcpConnectionPtr&)          // output buffer empty
CloseCallback         void(const TcpConnectionPtr&)          // internal cleanup
```

## Threading Components

### Thread (`src/base/thread.h`)
pthread wrapper with `CountDownLatch` for startup synchronization.

### EventLoopThread (`src/net/event_loop_thread.h`)
Owns a `Thread` and an `EventLoop`. Starts the thread and runs the event loop.

### EventLoopThreadPool (`src/net/event_loop_thread_pool.h`)
Manages multiple `EventLoopThread` objects.
- `SetThreadCount(n)` - Number of worker threads (0 = single-threaded, 1 = main loop only)
- `GetNextLoop()` - Round-robin distribution of connections

## Logging (`src/base/logging.h`)

```cpp
LOG_INFO << "message " << value;
LOG_ERROR << "Error: " << strerror(errno);
Logger::SetLevel(Logger::TRACE);
```

Log levels: `TRACE`, `DEBUG`, `INFO`, `WARN`, `ERROR`, `FATAL`

## Test Organization

Tests are numbered progressively (test01-test13 in `src/net/tests/`):

| Test | Feature |
|------|---------|
| test01 | Basic EventLoop |
| test04 | Timer scheduling |
| test05 | Cross-thread Execute/Submit |
| test07 | Acceptor |
| test08 | Single-threaded TcpServer |
| test09 | TcpConnection lifecycle |
| test10 | SIGPIPE handling |
| test13 | Multi-threaded server (4 threads) |

## Dependencies

- **Required:** Linux, pthread, Boost headers
- **Optional:** Protobuf, CURL, ZLIB

## Code Quality

- Compiler warnings: `-Wall -Wextra -Werror`
- Static analysis: `.clang-tidy` configuration
- Formatting: `.clang-format` configuration
