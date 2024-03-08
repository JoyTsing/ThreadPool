#include <future>

#include "minilog/minilog.h"
#include "task/task.h"
#include "thread/threadpool.h"

int main() {
  minilog::set_log_file("mini.log");
  minilog::set_log_level(minilog::log_level::warn);
  threadpool::ThreadPool pool;
  pool.setMode(threadpool::PoolMode::MODE_CACHED);
  pool.start(2);
  for (int i = 0; i < 50; i++) {
    pool.submit(fun1, i * 100);
  }
  std::vector<std::future<int> > results;
  for (int i = 0; i < 50; i++) {
    results.emplace_back(pool.submit(expensive_task, i, i + 1));
  }
  minilog::log_warn("===== commit all =====");
  for (auto&& result : results) {
    printf("expensive_task result: %d\n", result.get());
  }
  results.clear();
  for (int i = 0; i < 8; ++i) {
    results.emplace_back(pool.submit([i] {
      std::cout << " ======= sub " << i << " ========= " << std::endl;
      std::cout << "hello " << i << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(2));
      std::cout << "world " << i << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(1));
      return i * i;
    }));
  }
  minilog::log_warn(" =======  commit all 2 ========= ");
  for (auto&& result : results)
    std::cout << " future get:" << result.get() << ' ' << std::endl;
  pool.submit(timeout_task);
  return 0;
}