#define ANKERL_NANOBENCH_IMPLEMENT
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "thread/threadpool.h"

#include <doctest.h>
#include <nanobench.h>

#include <boost/asio.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/system/detail/error_code.hpp>

#include "minilog/minilog.h"
#include "queue/wait_strategy.h"

/**
 * @brief only when you need generate flame open
 */
// NOLINTNEXTLINE
// TEST_CASE("perf") {
//   threadpool::ThreadPool pool;
//   minilog::set_log_level(minilog::log_level::warn);
//   pool.setMode(threadpool::PoolMode::MODE_CACHED);
//   pool.start();
//   int iter = 0;
//   ankerl::nanobench::Bench().minEpochIterations(5000).run(
//       "thread-pool mode cached performance test", [&]() {
//         std::vector<std::future<int>> results;

//         for (int i = 0; i < 10000; i++) {
//           results.emplace_back(pool.submit([i]() { return 2 * i + 1; }));
//         }

//         for (int i = 0; i < 10000; i++) {
//           int res = results[i].get();
//           CHECK(res == 2 * i + 1);
//         }
//         // minilog::log_warn("epoch {}", iter++);
//       });
// }

/**
 * @brief test1
 *
 */

// NOLINTNEXTLINE
TEST_CASE("thread-pool test1") {
  boost::asio::thread_pool pool(5);
  minilog::set_log_level(minilog::log_level::warn);
  int iter = 0;
  // 100 times faster than mine, LOL
  ankerl::nanobench::Bench().minEpochIterations(100).run(
      "[test1] boost::asio::thread_pool speed test", [&]() {
        for (int i = 0; i < 10000; i++) {
          boost::asio::post(pool, [i]() {
            std::ostringstream ss;
            ss << "hello world" << i;
            return ss.str();
          });
        }
        pool.join();
        // minilog::log_warn("epoch {}", iter++);
      });
}

// NOLINTNEXTLINE
TEST_CASE("thread-pool test1") {
  threadpool::ThreadPool pool;
  minilog::set_log_level(minilog::log_level::warn);
  pool.setMode(threadpool::PoolMode::MODE_FIXED);
  pool.start();
  int iter = 0;
  ankerl::nanobench::Bench().minEpochIterations(100).run(
      "[test1] thread-pool mode fixed speed test", [&]() {
        for (int i = 0; i < 10000; i++) {
          pool.submit([i]() {
            std::ostringstream ss;
            ss << "hello world" << i;
            return ss.str();
          });
        }
        // minilog::log_warn("epoch {}", iter++);
      });
}

// NOLINTNEXTLINE
TEST_CASE("thread-pool test1") {
  threadpool::ThreadPool pool;
  minilog::set_log_level(minilog::log_level::warn);
  pool.setMode(threadpool::PoolMode::MODE_FIXED);
  pool.setQueueWaitStrategy(new wait_strategy::YieldWaitStrategy());
  pool.start();
  int iter = 0;
  ankerl::nanobench::Bench().minEpochIterations(100).run(
      "[test1] thread-pool mode fixed yield-waitStrategy speed test", [&]() {
        for (int i = 0; i < 10000; i++) {
          pool.submit([i]() {
            std::ostringstream ss;
            ss << "hello world" << i;
            return ss.str();
          });
        }
        // minilog::log_warn("epoch {}", iter++);
      });
}

// NOLINTNEXTLINE
TEST_CASE("thread-pool test1") {
  threadpool::ThreadPool pool;
  minilog::set_log_level(minilog::log_level::warn);
  pool.setMode(threadpool::PoolMode::MODE_CACHED);
  pool.start();
  int iter = 0;
  ankerl::nanobench::Bench().minEpochIterations(100).run(
      "[test1] thread-pool mode cached speed test", [&]() {
        for (int i = 0; i < 10000; i++) {
          pool.submit([i]() {
            std::ostringstream ss;
            ss << "hello world" << i;
            return ss.str();
          });
        }
        // minilog::log_warn("epoch {}", iter++);
      });
}

// NOLINTNEXTLINE
TEST_CASE("thread-pool test1") {
  threadpool::ThreadPool pool;
  minilog::set_log_level(minilog::log_level::warn);
  pool.setMode(threadpool::PoolMode::MODE_CACHED);
  pool.setQueueWaitStrategy(new wait_strategy::YieldWaitStrategy());
  pool.start();
  int iter = 0;
  ankerl::nanobench::Bench().minEpochIterations(100).run(
      "[test1] thread-pool mode cached speed test", [&]() {
        for (int i = 0; i < 10000; i++) {
          pool.submit([i]() {
            std::ostringstream ss;
            ss << "hello world" << i;
            return ss.str();
          });
        }
        // minilog::log_warn("epoch {}", iter++);
      });
}

/**
 * @brief test2
 *
 */

// NOLINTNEXTLINE
TEST_CASE("thread-pool test2") {
  threadpool::ThreadPool pool;
  minilog::set_log_level(minilog::log_level::warn);
  pool.setMode(threadpool::PoolMode::MODE_FIXED);
  pool.start();
  int iter = 0;
  ankerl::nanobench::Bench().minEpochIterations(100).run(
      "[test2] thread-pool mode FIXED performance test", [&]() {
        std::vector<std::future<int>> results;

        for (int i = 0; i < 10000; i++) {
          results.emplace_back(pool.submit([i]() { return 2 * i + 1; }));
        }

        for (int i = 0; i < 10000; i++) {
          int res = results[i].get();
          CHECK(res == 2 * i + 1);
        }
        // minilog::log_warn("epoch {}", iter++);
      });
}

// NOLINTNEXTLINE
TEST_CASE("thread-pool test2") {
  threadpool::ThreadPool pool;
  minilog::set_log_level(minilog::log_level::warn);
  pool.setMode(threadpool::PoolMode::MODE_FIXED);
  pool.setQueueWaitStrategy(new wait_strategy::YieldWaitStrategy());
  pool.start();
  int iter = 0;
  ankerl::nanobench::Bench().minEpochIterations(100).run(
      "[test2] thread-pool mode FIXED Yield WaitStrategy performance test",
      [&]() {
        std::vector<std::future<int>> results;

        for (int i = 0; i < 10000; i++) {
          results.emplace_back(pool.submit([i]() { return 2 * i + 1; }));
        }

        for (int i = 0; i < 10000; i++) {
          int res = results[i].get();
          CHECK(res == 2 * i + 1);
        }
        // minilog::log_warn("epoch {}", iter++);
      });
}

// NOLINTNEXTLINE
TEST_CASE("thread-pool test2") {
  threadpool::ThreadPool pool;
  minilog::set_log_level(minilog::log_level::warn);
  pool.setMode(threadpool::PoolMode::MODE_CACHED);
  pool.start();
  int iter = 0;
  ankerl::nanobench::Bench().minEpochIterations(100).run(
      "[test2] thread-pool mode cached performance test", [&]() {
        std::vector<std::future<int>> results;

        for (int i = 0; i < 10000; i++) {
          results.emplace_back(pool.submit([i]() { return 2 * i + 1; }));
        }

        for (int i = 0; i < 10000; i++) {
          int res = results[i].get();
          CHECK(res == 2 * i + 1);
        }
        // minilog::log_warn("epoch {}", iter++);
      });
}

// NOLINTNEXTLINE
TEST_CASE("thread-pool test2") {
  threadpool::ThreadPool pool;
  minilog::set_log_level(minilog::log_level::warn);
  pool.setMode(threadpool::PoolMode::MODE_CACHED);
  pool.setQueueWaitStrategy(new wait_strategy::YieldWaitStrategy());
  pool.start();
  int iter = 0;
  ankerl::nanobench::Bench().minEpochIterations(100).run(
      "[test2] thread-pool mode cached yield-waitStrategy performance test",
      [&]() {
        std::vector<std::future<int>> results;

        for (int i = 0; i < 10000; i++) {
          results.emplace_back(pool.submit([i]() { return 2 * i + 1; }));
        }

        for (int i = 0; i < 10000; i++) {
          int res = results[i].get();
          CHECK(res == 2 * i + 1);
        }
        // minilog::log_warn("epoch {}", iter++);
      });
}

// NOLINTNEXTLINE
TEST_CASE("thread-pool test2") {
  threadpool::ThreadPool pool;
  minilog::set_log_level(minilog::log_level::warn);
  pool.setMode(threadpool::PoolMode::MODE_CACHED);
  pool.setQueueWaitStrategy(new wait_strategy::TimeoutBlockStrategy());
  pool.start();
  int iter = 0;
  ankerl::nanobench::Bench().minEpochIterations(100).run(
      "[test2] thread-pool mode cached performance test", [&]() {
        std::vector<std::future<int>> results;

        for (int i = 0; i < 10000; i++) {
          results.emplace_back(pool.submit([i]() { return 2 * i + 1; }));
        }

        for (int i = 0; i < 10000; i++) {
          int res = results[i].get();
          CHECK(res == 2 * i + 1);
        }
        // minilog::log_warn("epoch {}", iter++);
      });
}

/**
 * @brief test3s
 *
 */

void func_swap(int& a, int& b) {
  int tmp = a;
  a = b;
  b = tmp;
}

auto test = [](Function<void(int&, int&)> func) {
  for (int i = 0; i < 10000; i++) {
    int should_a = i + 1, should_b = i;
    int a = i, b = i + 1;
    func(std::ref(a), std::ref(b));
    CHECK(a == should_a);
    CHECK(b == should_b);
  }
};

// NOLINTNEXTLINE
TEST_CASE("thread-pool test3") {
  boost::asio::thread_pool pool(5);
  minilog::set_log_level(minilog::log_level::warn);
  int iter = 0;
  ankerl::nanobench::Bench().minEpochIterations(5000).run(
      "[test3] boost::asio::thread_pool performance test3",
      [&]() { boost::asio::post(pool, [] { test(func_swap); }); });
}

// NOLINTNEXTLINE
TEST_CASE("thread-pool test3") {
  threadpool::ThreadPool pool;
  minilog::set_log_level(minilog::log_level::warn);
  pool.setMode(threadpool::PoolMode::MODE_FIXED);
  pool.start();
  int iter = 0;
  ankerl::nanobench::Bench().minEpochIterations(5000).run(
      "[test3] thread-pool mode FIXED performance test",
      [&]() { pool.submit(test, func_swap); });
}

// NOLINTNEXTLINE
TEST_CASE("thread-pool test3") {
  threadpool::ThreadPool pool;
  minilog::set_log_level(minilog::log_level::warn);
  pool.setMode(threadpool::PoolMode::MODE_FIXED);
  pool.setQueueWaitStrategy(new wait_strategy::YieldWaitStrategy());
  pool.start();
  int iter = 0;
  ankerl::nanobench::Bench().minEpochIterations(5000).run(
      "[test3] thread-pool fixed mode YieldWaitStrategy FIXED performance test",
      [&]() { pool.submit(test, func_swap); });
}

// NOLINTNEXTLINE
TEST_CASE("thread-pool test3") {
  threadpool::ThreadPool pool;
  minilog::set_log_level(minilog::log_level::warn);
  pool.setMode(threadpool::PoolMode::MODE_CACHED);
  pool.start();
  int iter = 0;
  ankerl::nanobench::Bench().minEpochIterations(5000).run(
      "[test3] thread-pool cached mode performance test",
      [&]() { pool.submit(test, func_swap); });
}

// NOLINTNEXTLINE
TEST_CASE("thread-pool test3") {
  threadpool::ThreadPool pool;
  minilog::set_log_level(minilog::log_level::warn);
  pool.setMode(threadpool::PoolMode::MODE_CACHED);
  pool.setQueueWaitStrategy(new wait_strategy::YieldWaitStrategy());
  pool.start();
  int iter = 0;
  ankerl::nanobench::Bench().minEpochIterations(5000).run(
      "[test3] thread-pool cached mode Yield-WaitStrategy performance test",
      [&]() { pool.submit(test, func_swap); });
}
