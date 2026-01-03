#ifndef MUDUO_BASE_LOGFILE_H
#define MUDUO_BASE_LOGFILE_H

#include <cstddef>
#include <mutex>
#include <string>

#include <memory>
#include "noncopyable.h"

namespace muduo {

using std::string;

class LogFile : NonCopyable {
 public:
  LogFile(string basename, size_t roll_size, bool thread_safe = true, int flush_interval = 3);
  ~LogFile();

  void Append(const char *logline, int len);
  void Flush();

 private:
  void AppendUnlocked(const char *logline, int len);
  static string GetLogFileName(const string &basename, time_t *now);

  void RollFile();

  const string basename_;
  const size_t roll_size_;
  const int flush_interval_;

  int count_;
  std::unique_ptr<std::mutex> mutex_;
  time_t start_of_period_;
  time_t last_roll_;
  time_t last_flush_;
  class File;
  std::unique_ptr<File> file_;

  const static int K_CHECK_TIME_ROLL = 1024;
  const static int K_ROLL_PER_SECONDS = 60 * 60 * 24;
};

}  // namespace muduo

#endif  // MUDUO_BASE_LOGFILE_H