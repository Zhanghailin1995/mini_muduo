//
// Created by hailin on 11/17/22.
//

#ifndef MINI_MUDUO_COPYABLE_H
#define MINI_MUDUO_COPYABLE_H

namespace muduo {

/// A tag class emphasises the objects are copyable.
/// The empty base class optimization applies.
/// Any derived class of copyable should be a value type.
class Copyable {
 protected:
  Copyable() = default;
  ~Copyable() = default;
};

}  // namespace muduo

#endif  // MINI_MUDUO_COPYABLE_H
