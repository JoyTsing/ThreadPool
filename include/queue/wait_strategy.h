#pragma once

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>

namespace wait_strategy {
class WaitStrategy {
 public:
  virtual void NotifyOne() {};
  virtual void BreakAllWait() {};
  virtual bool EmptyWait() = 0;
  virtual ~WaitStrategy() {};
};

/**
 * @brief 阻塞等待策略
 */

class BlockWaitStrategy : public WaitStrategy {
 public:
  BlockWaitStrategy() = default;
  void NotifyOne() override { cv_.notify_one(); }

  void BreakAllWait() override { cv_.notify_all(); }

  bool EmptyWait() override {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock);
    return true;
  }

 private:
  std::mutex mutex_;
  std::condition_variable cv_;
};

/**
 * @brief sleep等待策略
 *
 */

class SleepWaitStrategy : public WaitStrategy {
 public:
  SleepWaitStrategy() = default;
  explicit SleepWaitStrategy(std::uint64_t sleep_time_ms)
      : sleep_time_ms_(sleep_time_ms) {}

  bool EmptyWait() override {
    std::this_thread::sleep_for(std::chrono::microseconds(sleep_time_ms_));
    return true;
  }

  void set_sleep_time(std::uint64_t sleep_time_ms) {
    if (sleep_time_ms < 0) {
      sleep_time_ms = 1000;
    }
    sleep_time_ms_ = sleep_time_ms;
  }

 private:
  std::uint64_t sleep_time_ms_ = 1000;
};

/**
 * @brief yield等待策略
 *
 */

class YieldWaitStrategy : public WaitStrategy {
 public:
  YieldWaitStrategy() {}
  bool EmptyWait() override {
    // 让出cpu,节省资源
    std::this_thread::yield();
    return true;
  }
};

/**
 * @brief timeout等待策略
 * 结合定时器和阻塞等待，在一定时间内等到的处理和超时等到的处理相互分离
 */

class TimeoutBlockStrategy : public WaitStrategy {
 public:
  TimeoutBlockStrategy() = default;

  explicit TimeoutBlockStrategy(std::uint64_t timeout_ms)
      : timeout_ms_(timeout_ms) {}

  bool EmptyWait() override {
    std::unique_lock<std::mutex> lock(mutex_);
    if (cv_.wait_for(lock, timeout_ms_) == std::cv_status::timeout) {
      return false;
    }
    return true;
  }

  void NotifyOne() override { cv_.notify_one(); }

  void BreakAllWait() override { cv_.notify_all(); }

  void set_timeout(std::uint64_t timeout_us) {
    timeout_ms_ = std::chrono::milliseconds(timeout_us);
  }

 private:
  std::chrono::milliseconds timeout_ms_ = std::chrono::milliseconds(1000);
  std::condition_variable cv_;
  std::mutex mutex_;
};
}  // namespace wait_strategy