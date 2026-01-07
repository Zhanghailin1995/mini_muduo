//
// Created by hailin on 11/29/22.
//

#ifndef MUDUO_BASE_TIMESTAMP_H
#define MUDUO_BASE_TIMESTAMP_H

#include <cstdint>
#include <string>
#include "src/base/copyable.h"

namespace muduo {
class Timestamp : public muduo::Copyable {
 public:
  /// Constucts an invalid Timestamp.
  Timestamp();

  /// Constucts a Timestamp at specific time
  explicit Timestamp(int64_t micro_seconds_since_epoch);

  void Swap(Timestamp& that) {
    std::swap(micro_seconds_since_epoch_, that.micro_seconds_since_epoch_);
  }

  std::string ToString() const;
  std::string ToFormattedString() const;

  bool Valid() const { return micro_seconds_since_epoch_ > 0; }
  int64_t MicroSecondsSinceEpoch() const { return micro_seconds_since_epoch_; }

  static Timestamp Now();

  static Timestamp Invalid();

  static const int K_MICRO_SECONDS_PER_SECOND = 1000 * 1000;

 private:
  int64_t micro_seconds_since_epoch_;
};

inline bool operator<(Timestamp lhs, Timestamp rhs) {
  return lhs.MicroSecondsSinceEpoch() < rhs.MicroSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs) {
  return lhs.MicroSecondsSinceEpoch() == rhs.MicroSecondsSinceEpoch();
}

inline double TimeDifference(Timestamp high, Timestamp low) {
  int64_t diff = high.MicroSecondsSinceEpoch() - low.MicroSecondsSinceEpoch();
  return static_cast<double>(diff) / Timestamp::K_MICRO_SECONDS_PER_SECOND;
}

inline Timestamp AddTime(Timestamp timestamp, double seconds) {
  int64_t delta = static_cast<int64_t>(seconds * Timestamp::K_MICRO_SECONDS_PER_SECOND);
  return Timestamp(timestamp.MicroSecondsSinceEpoch() + delta);
}

}  // namespace muduo

#endif  // MUDUO_BASE_TIMESTAMP_H
