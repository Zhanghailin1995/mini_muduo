#include "log_file.h"
#include <unistd.h>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <memory>
#include <utility>

using namespace muduo;  // NOLINT

class LogFile::File : NonCopyable {
 public:
  explicit File(const string& filename) : fp_(::fopen(filename.c_str(), "ae")), written_bytes_(0) {
    ::setbuffer(fp_, buffer_, sizeof buffer_);
  }

  ~File() { ::fclose(fp_); }

  void Append(const char* logline, const size_t len) {
    size_t n = Write(logline, len);
    size_t remain = len - n;
    while (remain > 0) {
      size_t x = Write(logline + n, remain);
      if (x == 0) {
        int err = ferror(fp_);
        if (err != 0) {
          char buf[128];
          auto res = strerror_r(err, buf, sizeof buf);
          fprintf(stderr, "LogFile::File::Append() failed %s\n", res);
        }
        break;
      }
      n += x;
      remain = len - n;
    }

    written_bytes_ += len;
  }

  void Flush() {
    // printf("LogFile::File::Flush()\n");
    ::fflush(fp_);
  }

  size_t WrittenBytes() const { return written_bytes_; }

 private:
  size_t Write(const char* logline, size_t len) { return fwrite_unlocked(logline, 1, len, fp_); }

  FILE* fp_;
  char buffer_[64 * 1024];
  size_t written_bytes_;
};

LogFile::LogFile(string basename, size_t roll_size, bool thread_safe, int flush_interval)
    : basename_(std::move(basename)),
      roll_size_(roll_size),
      flush_interval_(flush_interval),
      count_(0),
      mutex_(thread_safe ? new std::mutex : nullptr),
      start_of_period_(0),
      last_roll_(0),
      last_flush_(0) {
  // printf("basename: %s\n", basename_.c_str());
  assert(basename.find('/') == string::npos);
  RollFile();
}

LogFile::~LogFile() = default;

void LogFile::Append(const char* logline, int len) {
  if (mutex_) {
    std::lock_guard<std::mutex> lock(*mutex_);
    AppendUnlocked(logline, len);
  } else {
    AppendUnlocked(logline, len);
  }
}

void LogFile::Flush() {
  if (mutex_) {
    std::lock_guard<std::mutex> lock(*mutex_);
    file_->Flush();
  } else {
    file_->Flush();
  }
}

void LogFile::AppendUnlocked(const char* logline, int len) {
  file_->Append(logline, static_cast<size_t>(len));
  if (file_->WrittenBytes() > roll_size_) {
    RollFile();
  } else {
    if (count_ >= K_CHECK_TIME_ROLL) {
      count_ = 0;
      time_t now = ::time(nullptr);
      time_t this_period = now / K_ROLL_PER_SECONDS * K_ROLL_PER_SECONDS;
      if (this_period != start_of_period_) {
        RollFile();
      } else if (now - last_flush_ > flush_interval_) {
        last_flush_ = now;
        file_->Flush();
      }
    } else {
      ++count_;
    }
  }
}

void LogFile::RollFile() {
  time_t now = 0;
  string filename = GetLogFileName(basename_, &now);
  time_t start = now / K_ROLL_PER_SECONDS * K_ROLL_PER_SECONDS;
  // printf("now: %ld, last_roll: %ld, start: %ld\n", now, last_roll_, start);
  // not roll file in same second
  if (now > last_roll_) {
    last_roll_ = now;
    last_flush_ = now;
    start_of_period_ = start;
    file_ = std::unique_ptr<File>(new File(filename));
  }
}

string LogFile::GetLogFileName(const string& basename, time_t* now) {
  string filename;
  filename.reserve(basename.size() + 32);
  filename = basename;

  char timebuf[32];
  struct tm tm{};
  *now = time(nullptr);
  gmtime_r(now, &tm);
  strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm);
  filename += timebuf;

  char pidbuf[32];
  snprintf(pidbuf, sizeof pidbuf, "%d", ::getpid());
  filename += pidbuf;

  filename += ".log";

  return filename;
}