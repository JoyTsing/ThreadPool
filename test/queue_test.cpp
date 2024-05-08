#include <doctest/doctest.h>
#include <nanobench.h>

#include <functional>
#include <future>
#include <iostream>
#include <thread>

#include "minilog/minilog.h"
#include "queue/lockfree_queue.h"
#include "queue/wait_strategy.h"

class ThreadPool {
 public:
  explicit ThreadPool(std::size_t thread_num, std::size_t max_task_num = 1000)
      : stop_(false) {
    // 初始化失败抛出异常
    if (!task_queue_.Init(max_task_num,
                          new wait_strategy::TimeoutBlockStrategy())) {
      throw std::runtime_error("Task queue init failed");
    }

    // 存放多个 std::thread线程对象
    workers_.reserve(thread_num);

    for (size_t i = 0; i < thread_num; ++i) {
      // 使用一个 lambda表达式来创建每个线程
      // 功能是 从任务队列中获取任务，并执行任务的函数对象
      workers_.emplace_back([this] {
        while (!stop_) {
          std::function<void()> task;
          if (task_queue_.wait_dequeue(&task)) {
            task();
          }
        }
      });
    }
  }

  template <typename F, typename... Args>
  auto Enqueue(F &&f, Args &&...args)
      -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;

    // 函数 f和其参数args， 打包成一个 std::packaged_task对象，放入任务队列
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    // 并返回一个与该任务关联的 std::future对象
    std::future<return_type> res = task->get_future();

    if (stop_) {
      return std::future<return_type>();
    }

    task_queue_.enqueue([task]() { (*task)(); });
    return res;
  }

  inline ~ThreadPool() {
    if (stop_.exchange(true)) {
      return;
    }

    task_queue_.BreakAllWait();

    for (std::thread &worker : workers_) {
      worker.join();
    }
  }

 private:
  std::vector<std::thread> workers_;
  BoundedQueue<std::function<void()>> task_queue_;
  std::atomic_bool stop_;
};

class Test_ThreadPool {
 public:
  void test() {
    ThreadPool thread_pool(4);
    std::vector<std::future<std::string>> results;

    for (int i = 0; i < 8; i++) {
      results.emplace_back(thread_pool.Enqueue([i]() {
        std::ostringstream ss;
        ss << "hello world" << i;
        std::cout << ss.str() << std::endl;
        return ss.str();
      }));
    }

    for (auto &&result : results) {
      std::cout << "result: " << result.get() << std::endl;
    }
  }
  void test2() {
    ThreadPool thread_pool(4);
    for (int i = 0; i < 10000; i++) {
      thread_pool.Enqueue([&]() {
        minilog::log_info(
            "num: {} ,thread {}", i,
            std::hash<std::thread::id>{}(std::this_thread::get_id()));
      });
    }
    return;
  }
};

// NOLINTNEXTLINE
TEST_CASE("lock-free queue test 1") {
  Test_ThreadPool test_;
  ankerl::nanobench::Bench().run("lock-free queue test", [&]() {
    for (int i = 0; i < 100; i++) {
      test_.test();
    }
  });
}

// NOLINTNEXTLINE
TEST_CASE("lock-free queue test 2") {
  Test_ThreadPool test_;
  ankerl::nanobench::Bench().run("memory and performance test", [&]() {
    for (int i = 0; i < 100; i++) {
      test_.test2();
      minilog::log_warn("epoch {}", i);
    }
  });
}