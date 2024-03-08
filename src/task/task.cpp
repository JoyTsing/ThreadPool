#include "task/task.h"

#include <chrono>
#include <thread>
int sum1(int a, int b) {
  std::this_thread::sleep_for(std::chrono::seconds(2));
  // 比较耗时
  return a + b;
}

int sum2(int a, int b, int c) {
  std::this_thread::sleep_for(std::chrono::seconds(2));
  return a + b + c;
}

// io线程
void io_thread(int listenfd) {}

// worker线程
void worker_thread(int clientfd) {}

int expensive_task(int a, int b) {
  std::this_thread::sleep_for(std::chrono::seconds(20));
  // 比较耗时
  return a + b;
}

void timeout_task() {
  std::this_thread::sleep_for(std::chrono::seconds(40));
  return;
}

void fun1(int slp) {
  // Convert std::thread::id to int using std::hash
  std::hash<std::thread::id> hashFunction;
  size_t hashValue = hashFunction(std::this_thread::get_id());
  // printf("  hello, fun1 !  %zu\n", hashValue);

  printf("  hello, fun1 ! \n");
  if (slp > 0) {
    printf(" ======= fun1 sleep %d  ========= \n", slp);
    std::this_thread::sleep_for(std::chrono::milliseconds(slp));
  }
}