#define ANKERL_NANOBENCH_IMPLEMENT
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest.h>
#include <nanobench.h>

short *dummy(int i) {
  short *ret = new short;
  return ret;
}

// NOLINTNEXTLINE
TEST_CASE("memoryhook-test") {
  ankerl::nanobench::Bench().minEpochIterations(50).run("combine-test", []() {
    for (int i = 0; i < 30000; i++) {
      short *p = dummy(i);
      delete p;
    }
  });
}

// NOLINTNEXTLINE
TEST_CASE("memoryhook-test") {
  dummy(3);
  int *p = new int;
  delete p;
  void *q = malloc(sizeof(int));
}