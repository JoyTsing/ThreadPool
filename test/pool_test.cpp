#include <doctest/doctest.h>
#include <nanobench.h>

#include <thread>

#include "minilog/minilog.h"
#include "thread/threadpool.h"

threadpool::ThreadPool pool;
void test(int k) {
  for (int i = 0; i < 10000; i++) {
    pool.submit([&]() {
      minilog::log_info(
          "num: {} ,thread {}", i,
          std::hash<std::thread::id>{}(std::this_thread::get_id()));
      // if (i == 9999) {
      //   minilog::log_warn("all submit");
      // }
    });
  }
  return;
}

// NOLINTNEXTLINE
TEST_CASE("thread-pool") {
  int numThreads = std::thread::hardware_concurrency();
  minilog::log_warn("max threads {}", numThreads);
  minilog::set_log_level(minilog::log_level::warn);

  pool.setMode(threadpool::PoolMode::MODE_FIXED);
  pool.start(8);
  ankerl::nanobench::Bench().run("dead-lock test", []() {
    for (int i = 0; i < 10; i++) {
      test(i);
      minilog::log_warn("epoch {}", i);
    }
  });
}
