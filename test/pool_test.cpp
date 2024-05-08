#include <doctest/doctest.h>
#include <nanobench.h>

#include "minilog/minilog.h"
#include "thread/threadpool.h"

// NOLINTNEXTLINE
TEST_CASE("thread-pool") {
  threadpool::ThreadPool pool;
  minilog::set_log_level(minilog::log_level::warn);
  pool.setMode(threadpool::PoolMode::MODE_CACHED);
  pool.start();
  int iter = 0;
  ankerl::nanobench::Bench().minEpochIterations(500).run(
      "thread-pool mode cached performance test", [&]() {
        std::vector<std::future<int>> results;

        for (int i = 0; i < 10000; i++) {
          results.emplace_back(pool.submit([i]() { return 2 * i + 1; }));
        }

        for (int i = 0; i < 10000; i++) {
          int res = results[i].get();
          CHECK(res == 2 * i + 1);
        }
        minilog::log_warn("epoch {}", iter++);
      });
}

// NOLINTNEXTLINE
TEST_CASE("thread-pool") {
  threadpool::ThreadPool pool;
  minilog::set_log_level(minilog::log_level::warn);
  pool.setMode(threadpool::PoolMode::MODE_FIXED);
  pool.start();
  int iter = 0;
  ankerl::nanobench::Bench().minEpochIterations(500).run(
      "thread-pool mode FIXED performance test", [&]() {
        std::vector<std::future<int>> results;

        for (int i = 0; i < 10000; i++) {
          results.emplace_back(pool.submit([i]() { return 2 * i + 1; }));
        }

        for (int i = 0; i < 10000; i++) {
          int res = results[i].get();
          CHECK(res == 2 * i + 1);
        }
        minilog::log_warn("epoch {}", iter++);
      });
}
