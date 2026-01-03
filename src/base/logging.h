#ifndef MUDUO_BASE_LOGGING_H
#define MUDUO_BASE_LOGGING_H

#include <memory>
#include "log_stream.h"

namespace muduo {
class Logger {
 public:
  enum LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    NUM_LOG_LEVELS,
  };

  class SourceFile {
   public:
    template <int N>
    SourceFile(const char (&arr)[N]) : data_(arr), size_(N - 1) {  // NOLINT
      const char *slash = strrchr(data_, '/');
      if (slash) {
        data_ = slash + 1;
        size_ -= static_cast<int>(data_ - arr);
      }
    }

    explicit SourceFile(const char *filename) : data_(filename) {
      const char *slash = strrchr(filename, '/');
      if (slash != nullptr) {
        data_ = slash + 1;
      }
      size_ = static_cast<int>(strlen(data_));
    }

    const char *data_;
    int size_;
  };

  Logger(SourceFile file, int line);
  Logger(SourceFile file, int line, LogLevel level);
  Logger(SourceFile file, int line, LogLevel level, const char *func);
  Logger(SourceFile file, int line, bool to_abort);
  ~Logger();

  LogStream &Stream() { return impl_.stream_; }

  static LogLevel GetLevel();
  static const char *GetLevelName();
  static void SetLevel(LogLevel level);

  using OutputFunc = void (*)(const char *, int);
  using FlushFunc = void (*)();
  static void SetOutput(OutputFunc);
  static void SetFlush(FlushFunc);

 private:
  class Impl {
   public:
    using LogLevel = Logger::LogLevel;
    Impl(LogLevel level, int saved_errno, const SourceFile &file, int line);
    void FormatTime();
    void Finish();

    int64_t time_;
    LogStream stream_;
    LogLevel level_;
    int line_;
    SourceFile basename_;
  };

  Impl impl_;
};

#define LOG_TRACE                                                  \
  if (muduo::Logger::GetLevel() <= muduo::Logger::LogLevel::TRACE) \
  muduo::Logger(__FILE__, __LINE__, muduo::Logger::LogLevel::TRACE, __func__).Stream()
#define LOG_DEBUG                                                  \
  if (muduo::Logger::GetLevel() <= muduo::Logger::LogLevel::TRACE) \
  muduo::Logger(__FILE__, __LINE__, muduo::Logger::LogLevel::DEBUG, __func__).Stream()
#define LOG_INFO \
  if (muduo::Logger::GetLevel() <= muduo::Logger::LogLevel::INFO) muduo::Logger(__FILE__, __LINE__).Stream()
#define LOG_WARN muduo::Logger(__FILE__, __LINE__, muduo::Logger::LogLevel::WARN).Stream()
#define LOG_ERROR muduo::Logger(__FILE__, __LINE__, muduo::Logger::LogLevel::ERROR).Stream()
#define LOG_FATAL muduo::Logger(__FILE__, __LINE__, muduo::Logger::LogLevel::FATAL).Stream()
#define LOG_SYSERR muduo::Logger(__FILE__, __LINE__, false).Stream()
#define LOG_SYSFATAL muduo::Logger(__FILE__, __LINE__, true).Stream()

const char *strerror_tl(int saved_errno);  // NOLINT

#define CHECK_NOTNULL(val) ::muduo::CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

template <typename T>
T *CheckNotNull(Logger::SourceFile file, int line, const char *names, T *ptr) {
  if (ptr == nullptr) {
    Logger(file, line, Logger::FATAL).Stream() << names;
  }
  return ptr;
}

}  // namespace muduo

#endif  // MUDUO_BASE_LOGGING_H