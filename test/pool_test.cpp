#define ANKERL_NANOBENCH_IMPLEMENT
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest.h>
#include <nanobench.h>

#include <boost/asio.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/system/detail/error_code.hpp>

#include "minilog/minilog.h"
#include "queue/wait_strategy.h"
#include "thread/threadpool.h"

// NOLINTNEXTLINE
TEST_CASE("thread-pool") {
  boost::asio::thread_pool pool(5);
  minilog::set_log_level(minilog::log_level::warn);
  int iter = 0;
  // 100 times faster than mine, LOL
  ankerl::nanobench::Bench().minEpochIterations(10).run(
      "boost::asio::thread_pool speed test", [&]() {
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
TEST_CASE("thread-pool") {
  threadpool::ThreadPool pool;
  minilog::set_log_level(minilog::log_level::warn);
  pool.setMode(threadpool::PoolMode::MODE_FIXED);
  pool.setQueueWaitStrategy(new wait_strategy::YieldWaitStrategy());
  pool.start();
  int iter = 0;
  ankerl::nanobench::Bench().minEpochIterations(10).run(
      "thread-pool mode fixed yield-waitStrategy speed test", [&]() {
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
TEST_CASE("thread-pool") {
  threadpool::ThreadPool pool;
  minilog::set_log_level(minilog::log_level::warn);
  pool.setMode(threadpool::PoolMode::MODE_FIXED);
  pool.setQueueWaitStrategy(new wait_strategy::BlockWaitStrategy());
  pool.start();
  int iter = 0;
  ankerl::nanobench::Bench().minEpochIterations(10).run(
      "thread-pool mode fixed block-waitStrategy speed test", [&]() {
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
TEST_CASE("thread-pool") {
  threadpool::ThreadPool pool;
  minilog::set_log_level(minilog::log_level::warn);
  pool.setMode(threadpool::PoolMode::MODE_CACHED);
  pool.start();
  int iter = 0;
  ankerl::nanobench::Bench().minEpochIterations(10).run(
      "thread-pool mode cached speed test", [&]() {
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
TEST_CASE("thread-pool") {
  threadpool::ThreadPool pool;
  minilog::set_log_level(minilog::log_level::warn);
  pool.setMode(threadpool::PoolMode::MODE_CACHED);
  pool.start();
  int iter = 0;
  ankerl::nanobench::Bench().minEpochIterations(10).run(
      "thread-pool mode cached performance test", [&]() {
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
TEST_CASE("thread-pool") {
  threadpool::ThreadPool pool;
  minilog::set_log_level(minilog::log_level::warn);
  pool.setMode(threadpool::PoolMode::MODE_CACHED);
  pool.setQueueWaitStrategy(new wait_strategy::YieldWaitStrategy());
  pool.start();
  int iter = 0;
  ankerl::nanobench::Bench().minEpochIterations(10).run(
      "thread-pool mode cached yield-waitStrategy performance test", [&]() {
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
TEST_CASE("thread-pool") {
  threadpool::ThreadPool pool;
  minilog::set_log_level(minilog::log_level::warn);
  pool.setMode(threadpool::PoolMode::MODE_FIXED);
  pool.start();
  int iter = 0;
  ankerl::nanobench::Bench().minEpochIterations(10).run(
      "thread-pool mode FIXED performance test", [&]() {
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
