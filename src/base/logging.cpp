#include "logging.h"
#include "current_thread.h"
#include "log_stream.h"

#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstring>

namespace muduo {

__thread char t_errnobuf[512];
__thread char t_time[32];
__thread time_t t_last_second;

const char *strerror_tl(int saved_errno) { return strerror_r(saved_errno, t_errnobuf, sizeof t_errnobuf); }

Logger::LogLevel InitLogLevel() {
  if (::getenv("MUDUO_LOG_TRACE") != nullptr) {
    return Logger::LogLevel::TRACE;
  }
  if (::getenv("MUDUO_LOG_DEBUG") != nullptr) {
    return Logger::LogLevel::DEBUG;
  }
  return Logger::LogLevel::INFO;
}

Logger::LogLevel g_log_level = InitLogLevel();

const char *log_level_name[Logger::LogLevel::NUM_LOG_LEVELS] = {
    "TRACE ", "DEBUG ", "INFO  ", "WARN  ", "ERROR ", "FATAL ",
};

void DefaultOutput(const char *msg, int len) { fwrite(msg, 1, static_cast<size_t>(len), stdout); }

void DefaultFlush() { fflush(stdout); }

Logger::OutputFunc g_output = DefaultOutput;
Logger::FlushFunc g_flush = DefaultFlush;

inline LogStream &operator<<(LogStream &s, const Logger::SourceFile &v) {
  s.Append(v.data_, v.size_);
  return s;
}

}  // namespace muduo

using namespace muduo;  // NOLINT

int64_t MicroSecondsSinceEpoch() {
  auto now = std::chrono::system_clock::now();
  auto duration = now.time_since_epoch();
  return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}

Logger::Impl::Impl(LogLevel level, int saved_errno, const SourceFile &file, int line)
    : time_(MicroSecondsSinceEpoch()), stream_(), level_(level), line_(line), basename_(file) {
  FormatTime();
  current_thread::Tid();
  stream_ << StringHelper(current_thread::TidString(), current_thread::TidStringLength());
  stream_ << StringHelper(log_level_name[level], 6);
  if (saved_errno != 0) {
    stream_ << strerror_tl(saved_errno) << " (errno=" << saved_errno << ") ";
  }
}

void Logger::Impl::FormatTime() {
  int64_t micro_seconds_since_epoch = time_;
  time_t seconds = static_cast<time_t>(micro_seconds_since_epoch / 1000000);
  int microseconds = static_cast<int>(micro_seconds_since_epoch % 1000000);
  if (seconds != t_last_second) {
    t_last_second = seconds;
    struct tm tm_time;
    ::gmtime_r(&seconds, &tm_time);
    int len = snprintf(t_time, sizeof t_time, "%4d-%02d-%02d %02d:%02d:%02d", tm_time.tm_year + 1900,
                       tm_time.tm_mon + 1, tm_time.tm_mday, tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    // printf("t_time: %s, len: %d\n", t_time, len);
    assert(len == 19);
    (void)len;
  }
  Fmt us(".%06dZ ", microseconds);
  stream_ << StringHelper(t_time, 19) << StringHelper(us.Data(), 9);
}

void Logger::Impl::Finish() { stream_ << " - " << basename_ << ':' << line_ << "\n"; }

Logger::Logger(SourceFile file, int line) : impl_(LogLevel::INFO, 0, file, line) {}

Logger::Logger(SourceFile file, int line, LogLevel level, const char *func) : impl_(level, 0, file, line) {
  impl_.stream_ << "func: (" << func << ')' << ' ';
}

Logger::Logger(SourceFile file, int line, LogLevel level) : impl_(level, 0, file, line) {}

Logger::Logger(SourceFile file, int line, bool to_abort)
    : impl_(to_abort ? LogLevel::FATAL : LogLevel::ERROR, errno, file, line) {}

Logger::~Logger() {
  impl_.Finish();
  // printf("Logger deconstructor %p\n", this);
  const LogStream::Buffer &buf(Stream().InnerBuffer());
  g_output(buf.Data(), buf.Length());
  if (impl_.level_ == LogLevel::FATAL) {
    g_flush();
    abort();
  }
}

void Logger::SetLevel(Logger::LogLevel level) { g_log_level = level; }

Logger::LogLevel Logger::GetLevel() { return g_log_level; }

const char *Logger::GetLevelName() { return log_level_name[g_log_level]; }

void Logger::SetOutput(OutputFunc out) { g_output = out; }

void Logger::SetFlush(FlushFunc flush) { g_flush = flush; }