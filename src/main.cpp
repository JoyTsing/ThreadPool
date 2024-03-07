#include "minilog/minilog.h"
#include "thread/threadpool.h"

int main() {
  minilog::log_info("hello world : {}", 42);
  return 0;
}