#include <functional>
#define ANKERL_NANOBENCH_IMPLEMENT
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest.h>
#include <nanobench.h>

#include "function/function.h"

void func_hello(int i) { printf("#%d Hello\n", i); }

void func_swap(int& a, int& b) {
  int tmp = a;
  a = b;
  b = tmp;
}

// NOLINTNEXTLINE
TEST_CASE("function test") {
  auto test = [](Function<void(int)> func) {
    for (int i = 0; i < 10; i++) {
      func(i);
    }
  };
  ankerl::nanobench::Bench().minEpochIterations(1).run(
      "function test hello world", [&]() { test(func_hello); });
}

// NOLINTNEXTLINE
TEST_CASE("function test") {
  auto test = [](Function<void(int&, int&)> func) {
    for (int i = 0; i < 10000; i++) {
      int should_a = i + 1, should_b = i;
      int a = i, b = i + 1;
      func(std::ref(a), std::ref(b));
      CHECK(a == should_a);
      CHECK(b == should_b);
    }
  };
  ankerl::nanobench::Bench().minEpochIterations(500).run(
      "function test2", [&]() { test(func_swap); });
}
