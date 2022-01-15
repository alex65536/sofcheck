// This file is part of SoFCheck
//
// Copyright (c) 2022 Alexander Kernozhitsky and SoFCheck contributors
//
// SoFCheck is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SoFCheck is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SoFCheck.  If not, see <https://www.gnu.org/licenses/>.

#ifndef SOF_UTIL_QUEUE_INCLUDED
#define SOF_UTIL_QUEUE_INCLUDED

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <optional>
#include <queue>

namespace SoFUtil {

// Buffered blocking queue
template <typename T>
class BlockingQueue {
public:
  explicit BlockingQueue(size_t size) : size_(size) {}

  // Pushes `value` into the queue. Returns `true` if the value is pushed successfully, and `false`
  // if the value is not pushed because the queue is closed
  bool push(T value) {
    std::unique_lock lock(mutex_);
    fullEvent_.wait(lock, [this]() { return closed_ || !isQueueFull(); });
    if (closed_) {
      return false;
    }
    queue_.emplace(std::move(value));
    lock.unlock();
    emptyEvent_.notify_one();
    fullEvent_.notify_one();
    return true;
  }

  // Pops value from the queue. If the queue is empty, blocks until either it becomes non-empty or
  // closes. Returns `std::nullopt` if the queue is closed and empty, otherwise returns popped value
  std::optional<T> pop() {
    std::unique_lock lock(mutex_);
    emptyEvent_.wait(lock, [this]() { return closed_ || !queue_.empty(); });
    if (queue_.empty() && closed_) {
      return std::nullopt;
    }
    auto result = std::make_optional(std::move(queue_.front()));
    queue_.pop();
    lock.unlock();
    emptyEvent_.notify_one();
    fullEvent_.notify_one();
    return result;
  }

  // Closes the queue
  void close() {
    std::unique_lock lock(mutex_);
    if (closed_) {
      return;
    }
    closed_ = true;
    lock.unlock();
    emptyEvent_.notify_all();
    fullEvent_.notify_all();
  }

private:
  bool isQueueFull() const { return queue_.size() == size_; }

  std::queue<T> queue_;
  std::condition_variable emptyEvent_;
  std::condition_variable fullEvent_;
  std::mutex mutex_;
  size_t size_;
  bool closed_ = false;
};

}  // namespace SoFUtil

#endif  // SOF_UTIL_QUEUE_INCLUDED
