#pragma once
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "minilog/minilog.h"
#include "queue/lockfree_queue.h"
#include "queue/wait_strategy.h"
namespace threadpool {
namespace config {
static constexpr const int TASK_MAX_THRESHOLD = 60;
static constexpr const int THREAD_MAX_THRESHOLD = 12;
static constexpr const int THREAD_MAX_IDLE_SECOND = 3;
}  // namespace config

enum class PoolMode : uint8_t {
  MODE_FIXED,   // 固定数量
  MODE_CACHED,  // 动态增长
};

class Thread {
 public:
  using Task = std::function<void(int)>;  // 任务类型
  Thread(Task func);
  ~Thread() = default;
  void start();
  int getID() const;

 private:
  static int generatedId_;

 private:
  Task func_;
  int threadID_;
};

class ThreadPool {
 public:
  using Task = std::function<void()>;
  ThreadPool();
  ~ThreadPool();
  // non-copy
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  void setQueueWaitStrategy(wait_strategy::WaitStrategy* strategy);
  void setMode(PoolMode mod);
  void setTaskThreshold(int threshold);
  void setThreadThreshold(int threshold);

  // 提交task
  template <class F, class... Args>
  auto submit(F&& f, Args&&... args)
      -> std::future<typename std::invoke_result_t<F, Args...>> {
    using RetType = typename std::invoke_result_t<F, Args...>;
    auto task = std::make_shared<std::packaged_task<RetType()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    std::future<RetType> result = task->get_future();

    // lock
    std::unique_lock<std::mutex> lock(mtx_);
    // submit
    while (!notFull_.wait_for(lock, std::chrono::seconds(1), [&]() -> bool {
      return TaskQueue_.size() < taskThreshold_;
    })) {
      minilog::log_warn(
          "task queue is full,thread id {} submit task fail, try again",
          convertThreadId(std::this_thread::get_id()));
    }

    // push task
    // minilog::log_info("submit task to thread pool.");
    TaskQueue_.enqueue([task]() { (*task)(); });
    taskSize_++;
    // 通知分配线程执行任务
    notEmpty_.notify_all();

    // cached模式，根据任务数量和空闲线程的数量，判断是否需要创建新的线程出来
    if (mod_ == PoolMode::MODE_CACHED && taskSize_ > idleThreadSize_ &&
        curTheadSize_ < threadSizeThreshHold_) {
      // 创建新的线程
      auto thread_ptr = std::make_unique<Thread>(
          std::bind(&ThreadPool::newThread, this, std::placeholders::_1));
      int threadID = thread_ptr->getID();
      // push to pool
      pool_.emplace(threadID, std::move(thread_ptr));
      // minilog::log_info("create new thread, id {}.", threadID);
      pool_[threadID]->start();
      curTheadSize_++;
      idleThreadSize_++;
    }
    return result;
  }

  void start(int initThreadSize = std::thread::hardware_concurrency() / 4);

 private:
  void newThread(int threadid);
  bool isRunning() const;
  uint32_t convertThreadId(std::thread::id id);

 private:
  // init
  int initThreadSize_;
  int threadSizeThreshHold_;
  std::unordered_map<int, std::unique_ptr<Thread>> pool_;
  // thread size
  std::atomic<int> curTheadSize_;
  std::atomic<int> idleThreadSize_;
  // task queue
  // std::queue<Task> TaskQueue_;
  BoundedQueue<Task> TaskQueue_;

  std::atomic<int> taskSize_;
  int taskThreshold_;
  // thread safe
  std::mutex mtx_;
  std::condition_variable notFull_;
  std::condition_variable notEmpty_;
  std::condition_variable exit_;  // safe exit

  PoolMode mod_;
  std::atomic<bool> running_;
};

};  // namespace threadpool