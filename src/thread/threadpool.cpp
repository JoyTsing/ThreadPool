#include "thread/threadpool.h"

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

#include "minilog/minilog.h"

namespace threadpool {

ThreadPool::ThreadPool()
    : initThreadSize_(0),
      taskSize_(0),
      idleThreadSize_(0),
      curTheadSize_(0),
      taskThreshold_(config::TASK_MAX_THRESHOLD),
      threadSizeThreshHold_(config::THREAD_MAX_THRESHOLD),
      mod_(PoolMode::MODE_FIXED),
      running_(false) {
  TaskQueue_.Init(config::TASK_MAX_THRESHOLD);
}

ThreadPool::~ThreadPool() {
  running_ = false;
  std::unique_lock<std::mutex> lock(mtx_);
  notEmpty_.notify_all();
  exit_.wait(lock, [&]() -> bool { return pool_.size() == 0; });
  // minilog::log_info("Thread Pool destroy.");
}

void ThreadPool::setMode(PoolMode mod) {
  if (isRunning()) {
    return;
  }
  mod_ = mod;
}

void ThreadPool::setTaskThreshold(int threshold) {
  if (isRunning()) {
    return;
  }
  taskThreshold_ = threshold;
}

void ThreadPool::setThreadThreshold(int threshold) {
  if (isRunning()) {
    return;
  }
  if (mod_ == PoolMode::MODE_CACHED) {
    threadSizeThreshHold_ = threshold;
  }
}

void ThreadPool::setQueueWaitStrategy(wait_strategy::WaitStrategy* strategy) {
  if (isRunning()) {
    return;
  }
  TaskQueue_.set_waitStrategy(strategy);
}

void ThreadPool::start(int initThreadSize) {
  if (isRunning()) {
    minilog::log_warn("Thread Pool is started");
    return;
  }
  running_ = true;
  // 创建初始线程
  initThreadSize_ = initThreadSize;
  curTheadSize_ = initThreadSize;
  for (int i = 0; i < initThreadSize_; i++) {
    auto thread_ptr =
        std::make_unique<Thread>(std::bind(&ThreadPool::newThread, this, std::placeholders::_1));
    int threadID = thread_ptr->getID();
    pool_.emplace(threadID, std::move(thread_ptr));
    // minilog::log_info("create new thread, id {}.", threadID);
  }
  // 启动线程
  for (const auto& [id, thread] : pool_) {
    idleThreadSize_++;  // 空闲线程数量
    // minilog::log_info("thread id {} start.", id);
    thread->start();
  }
}

void ThreadPool::newThread(int threadid) {
  auto baseline = std::chrono::high_resolution_clock::now();
  uint32_t tid = convertThreadId(std::this_thread::get_id());
  while (true) {
    Task task;
    {
      std::unique_lock<std::mutex> lock(mtx_);
      // minilog::log_info("tid: {} try get task", tid);
      //  cached下，可能动态创建许多线程，但是如果空闲时间超过1min，就会被销毁
      //  主要销毁多出去threshold的线程

      while (TaskQueue_.empty()) {
        if (!running_) {
          pool_.erase(threadid);
          // minilog::log_info("thread id: {} exit.", threadid);
          exit_.notify_all();
          return;
        }

        if (mod_ == PoolMode::MODE_CACHED) {
          // timeout
          if (std::cv_status::timeout == notEmpty_.wait_for(lock, std::chrono::seconds(1))) {
            // heart-beat check
            auto now = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - baseline);
            if (duration.count() >= config::THREAD_MAX_IDLE_SECOND &&
                curTheadSize_ > initThreadSize_) {
              // recycle
              minilog::log_warn("thread id: {} timeout,destroy", threadid);
              pool_.erase(threadid);
              curTheadSize_--;
              idleThreadSize_--;
              return;
            }
          }
        }
        if (mod_ == PoolMode::MODE_FIXED) {
          notEmpty_.wait(lock);
        }
      }
      // minilog::log_info("tid: {} get Task", tid);
      // get task
      TaskQueue_.dequeue(&task);
      taskSize_--;
      if (!TaskQueue_.empty()) {
        notEmpty_.notify_all();
      }
      // 取出一个任务，进行通知，通知可以继续提交生产任务
      notFull_.notify_all();
    }
    // 执行任务 minilog::log_info("tid: {} do task", tid);
    idleThreadSize_--;
    if (task != nullptr) {
      task();
    }
    idleThreadSize_++;
    baseline = std::chrono::high_resolution_clock::now();  // 更新时间
  }
}

uint32_t ThreadPool::convertThreadId(std::thread::id id) {
  std::hash<std::thread::id> hasher;
  return static_cast<uint32_t>(hasher(id));
}

bool ThreadPool::isRunning() const { return running_; }
}  // namespace threadpool