#define ANKERL_NANOBENCH_IMPLEMENT
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest.h>
#include <optional>
#include <nanobench.h>

#include "optional/optional.h"
#include "minilog/minilog.h"

class TestClass_1 {
  public:
  TestClass_1(int x, int y)
      : m_x(x),
        m_y(y) {}
  int m_x, m_y;
};

// NOLINTNEXTLINE
TEST_CASE("Optional Function Test") {
  // check normal value
  Optional<int> opt_normal(1);
  CHECK(opt_normal.has_value() == true);
  CHECK(opt_normal.value() == 1);
  // check std::nullopt is the same as we write nullopt
  Optional<int> opt(nullopt);
  CHECK(opt.has_value() == false);
  Optional<int> std_opt(std::nullopt);
  CHECK(std_opt.has_value() == false);
  // check std::bad_optional_access is the same as our BadOptionalAccess
  Optional<int> opt_exception_std(std::nullopt);
  CHECK_THROWS_AS(opt_exception_std.value(), std::bad_optional_access);
  CHECK_THROWS_AS(opt_exception_std.value(), BadOptionalAccess);
  // check constructor with class
  Optional<TestClass_1> opt_class(TestClass_1(1, 2));
  CHECK(opt_class.has_value() == true);
  CHECK(opt_class.value().m_x == 1);
  CHECK(opt_class.value().m_y == 2);
  Optional<TestClass_1> opt_class_nullopt(nullopt);
  CHECK(opt_class_nullopt.has_value() == false);
  CHECK_THROWS_AS(opt_class_nullopt.value(), BadOptionalAccess);
  // check constructor with std::nullopt
  Optional<TestClass_1> opt_class_std_nullopt(std::nullopt);
  CHECK(opt_class_std_nullopt.has_value() == false);
  CHECK_THROWS_AS(opt_class_std_nullopt.value(), BadOptionalAccess);
  // check move
  Optional<TestClass_1> opt_class_move(TestClass_1(1, 2));
  auto moved = std::move(opt_class_move).value();
  CHECK(moved.m_x == 1);
  CHECK(moved.m_y == 2);
  Optional<TestClass_1> opt_class_move_2(TestClass_1(1, 2));
  auto moved_2 = std::move(opt_class_move_2.value());
  CHECK(moved_2.m_x == 1);
  CHECK(moved_2.m_y == 2);
  // check value_or
  Optional<int> opt_value_or(1);
  CHECK(opt_value_or.value_or(2) == 1);
  Optional<int> opt_value_or_nullopt(nullopt);
  CHECK(opt_value_or_nullopt.value_or(2) == 2);
  Optional<int> opt_value_or_std_nullopt(std::nullopt);
  CHECK(opt_value_or_std_nullopt.value_or(2) == 2);
}
// bench
// ankerl::nanobench::Bench().minEpochIterations(1000).run("optional test", [&]() {});