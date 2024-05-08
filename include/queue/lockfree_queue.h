#pragma once

#include <atomic>
#include <cstdint>
#include <memory>

#include "wait_strategy.h"

template <typename T>
class BoundedQueue {
 public:
  BoundedQueue() = default;
  ~BoundedQueue();
  BoundedQueue(const BoundedQueue&) = delete;
  BoundedQueue& operator=(const BoundedQueue&) = delete;

  bool Init(std::uint64_t cap);
  bool Init(std::uint64_t cap, wait_strategy::WaitStrategy*);

  bool enqueue(const T& item);
  bool dequeue(T* item);

  /**
   * @brief 队列满时，按照策略等待
   *
   * @param item
   * @return true
   * @return false
   */
  bool wait_enqueue(const T& item);

  /**
   * @brief 队列空时，按照策略等待
   *
   * @param item
   * @return true
   * @return false
   */
  bool wait_dequeue(T* item);

  // notify all thread to break wait
  void BreakAllWait();

 public:
  std::uint64_t get_head() { return head_.load(); }

  std::uint64_t get_tail() { return tail_.load(); }

  std::uint64_t get_max_head() { return max_head_.load(); }

  std::uint64_t size() { return tail_ - head_ - 1; }

  bool empty() { return size() == 0; }

  void set_waitStrategy(wait_strategy::WaitStrategy* wait_strategy) {
    wait_strategy_.reset(wait_strategy);
  }

 private:
  // 获取下标
  std::uint64_t get_index(std::uint64_t);

 private:
// 指定内存对齐方式, 可提高代码性能和效率
#define CACHELINE_SIZE 64
  alignas(CACHELINE_SIZE) std::atomic<std::uint64_t> head_ = {0};
  alignas(CACHELINE_SIZE) std::atomic<std::uint64_t> tail_ = {1};
  alignas(CACHELINE_SIZE) std::atomic<std::uint64_t> max_head_ = {
      1};  // 最大的head, tail的备份
#undef CACHELINE_SIZE

  std::uint64_t size_ = 0;
  T* pool_ = nullptr;
  std::unique_ptr<wait_strategy::WaitStrategy> wait_strategy_ = nullptr;
  volatile bool break_all_wait_ = false;
};

template <typename T>
inline std::uint64_t BoundedQueue<T>::get_index(std::uint64_t num) {
  return num - (num / size_) * size_;
}

template <typename T>
inline void BoundedQueue<T>::BreakAllWait() {
  break_all_wait_ = true;
  wait_strategy_->BreakAllWait();
}

template <typename T>
BoundedQueue<T>::~BoundedQueue() {
  BreakAllWait();
  if (pool_) {
    for (std::uint64_t i = 0; i < size_; i++) {
      pool_[i].~T();
    }
    std::free(pool_);
  }
}
template <typename T>
bool BoundedQueue<T>::Init(std::uint64_t cap) {
  return Init(cap, new wait_strategy::TimeoutBlockStrategy());
}

template <typename T>
bool BoundedQueue<T>::Init(std::uint64_t cap,
                           wait_strategy::WaitStrategy* wait_strategy) {
  size_ = cap + 2;
  pool_ = reinterpret_cast<T*>(std::calloc(size_, sizeof(T)));
  if (pool_ == nullptr) {
    return false;
  }
  for (std::uint64_t i = 0; i < size_; i++) {
    new (&pool_[i]) T();
  }
  wait_strategy_.reset(wait_strategy);
  return true;
}

template <typename T>
bool BoundedQueue<T>::wait_enqueue(const T& item) {
  while (!break_all_wait_) {
    if (enqueue(item)) {
      return true;
    }
    // 插入失败
    if (wait_strategy_->EmptyWait()) {
      continue;
    }
    break;
  }
  return false;
}

template <typename T>
bool BoundedQueue<T>::wait_dequeue(T* item) {
  while (!break_all_wait_) {
    if (dequeue(item)) {
      return true;
    }
    if (wait_strategy_->EmptyWait()) {
      continue;
    }
    break;
  }
  return false;
}

template <typename T>
bool BoundedQueue<T>::enqueue(const T& item) {
  std::uint64_t new_tail = 0;
  std::uint64_t old_tail = tail_.load(std::memory_order_acquire);  // 获取旧值
  std::uint64_t old_max_head = 0;
  do {
    new_tail = old_tail + 1;
    if (get_index(new_tail) ==
        get_index(head_.load(std::memory_order_acquire))) {
      return false;  // 队列满
    }
  } while (!tail_.compare_exchange_weak(old_tail, new_tail,
                                        std::memory_order_acq_rel,
                                        std::memory_order_relaxed));

  pool_[get_index(old_tail)] = item;

  do {
    old_max_head = old_tail;
  } while (!max_head_.compare_exchange_weak(old_tail, new_tail,
                                            std::memory_order_acq_rel,
                                            std::memory_order_relaxed));
  wait_strategy_->NotifyOne();
  return true;
}

template <typename T>
bool BoundedQueue<T>::dequeue(T* item) {
  std::uint64_t new_head = 0;
  std::uint64_t old_head = head_.load(std::memory_order_acquire);
  do {
    new_head = old_head + 1;
    // 是否在最大出队的下标之前，不用tail_，因为tail_可能会和入队操作冲突，
    // 也就是申请了空间但是元素还没正式插入
    if (new_head == max_head_.load(std::memory_order_acquire)) {
      return false;
    }
    *item = pool_[get_index(new_head)];
  } while (!head_.compare_exchange_weak(old_head, new_head,
                                        std::memory_order_acq_rel,
                                        std::memory_order_relaxed));
  wait_strategy_->NotifyOne();
  return true;
}
