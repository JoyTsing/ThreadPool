#include <utility>
#include <vector>
#define ANKERL_NANOBENCH_IMPLEMENT
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest.h>
#include <optional>
#include <nanobench.h>

#include "optional/optional.h"

class TestClass_1 {
  public:
  TestClass_1(int x, int y)
      : m_x(x),
        m_y(y) {}
  int m_x, m_y;
  int value_x() const { return m_x; }
};

class TestClass_info {
  public:
  TestClass_info(int x, int y)
      : m_x(x),
        m_y(y) {}
  int m_x, m_y;
  TestClass_info(TestClass_info const &) { printf("TestClass_info(TestClass_info const &)\n"); }

  TestClass_info(TestClass_info &&) { printf("TestClass_info(TestClass_info &&)\n"); }

  TestClass_info &operator=(TestClass_info const &) {
    printf("TestClass_info &operator=(TestClass_info const &)\n");
    return *this;
  }

  TestClass_info &operator=(TestClass_info &&) {
    printf("TestClass_info &operator=(TestClass_info &&)\n");
    return *this;
  }

  ~TestClass_info() { printf("~TestClass_info()\n"); }
};

// NOLINTNEXTLINE
TEST_CASE("Optional Base Test") {
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
  CHECK_THROWS_AS(opt_class_std_nullopt.value(), std::bad_optional_access);
  // check move
  Optional<TestClass_1> opt_class_move(TestClass_1(1, 2));
  auto moved = std::move(opt_class_move).value();
  CHECK(moved.m_x == 1);
  CHECK(moved.m_y == 2);
  Optional<TestClass_1> opt_class_move_2(TestClass_1(1, 2));
  auto moved_2 = std::move(opt_class_move_2.value());
  CHECK(moved_2.m_x == 1);
  CHECK(moved_2.m_y == 2);
  // check reset
  Optional<int> opt_reset(1);
  opt_reset.reset();
  CHECK(opt_reset.has_value() == false);
  CHECK_THROWS_AS(opt_reset.value(), std::bad_optional_access);
  // check value_or
  Optional<int> opt_value_or(1);
  CHECK(opt_value_or.value_or(2) == 1);
  Optional<int> opt_value_or_nullopt(nullopt);
  CHECK(opt_value_or_nullopt.value_or(2) == 2);
  Optional<int> opt_value_or_std_nullopt(std::nullopt);
  CHECK(opt_value_or_std_nullopt.value_or(2) == 2);
  // check assign nullopt and recover
  Optional<int> opt_assign(1);
  opt_assign = nullopt;
  CHECK(opt_assign.has_value() == false);
  CHECK_THROWS_AS(opt_assign.value(), BadOptionalAccess);
  opt_assign = std::nullopt;
  CHECK(opt_assign.has_value() == false);
  CHECK_THROWS_AS(opt_assign.value(), std::bad_optional_access);
  CHECK(opt_assign.value_or(2) == 2);
  opt_assign = 42;
  CHECK(opt_assign.has_value() == true);
  CHECK(opt_assign.value() == 42);
  // check copy
  opt_assign = Optional<int>(-42);
  CHECK(opt_assign.has_value() == true);
  CHECK(opt_assign.value() == -42);
  // check copy constructor
  opt_assign = Optional<int>(std::nullopt);
  CHECK(opt_assign.has_value() == false);
  // check operator
  Optional<TestClass_1> opt_operator({1, 2});
  CHECK(opt_operator->value_x() == 1);
  // check in place
  Optional<TestClass_1> opt_in_place(std::in_place, 1, 2);
  CHECK(opt_in_place.has_value() == true);
  CHECK(opt_in_place.value().m_x == 1);
  CHECK(opt_in_place.value().m_y == 2);
  // check in place with initializer list
  Optional<std::vector<int>> opt_in_place_list(std::in_place, {1, 2});
  CHECK(opt_in_place_list.has_value() == true);
  CHECK(opt_in_place_list.value()[0] == 1);
  CHECK(opt_in_place_list.value()[1] == 2);
  // check in place with our in_place
  Optional<TestClass_1> opt_in_place_our(in_place, 1, 2);
  CHECK(opt_in_place_our.has_value() == true);
  CHECK(opt_in_place_our.value().m_x == 1);
  CHECK(opt_in_place_our.value().m_y == 2);
  // check in place with initializer list
  Optional<std::vector<int>> opt_in_place_list_our(in_place, {1, 2});
  CHECK(opt_in_place_list_our.has_value() == true);
  CHECK(opt_in_place_list_our.value()[0] == 1);
  CHECK(opt_in_place_list_our.value()[1] == 2);
}

Optional<int> parserInt(std::string const &str) {
  try {
    return std::stoi(str);
  } catch (...) {
    return nullopt;
  }
}

// NOLINTNEXTLINE
TEST_CASE("Optional Function Test") {
  // cost move assignment
  // Optional<TestClass_info> opt_cost({1, 2});
  // check emplace, should just print destructor
  Optional<TestClass_info> opt_emplace = std::nullopt;
  opt_emplace.emplace(1, 2);
  std::optional<TestClass_info> std_opt_emplace = std::nullopt;
  std_opt_emplace.emplace(1, 2);
  auto opt_parser = parserInt("123");
  CHECK(opt_parser.has_value() == true);
  CHECK(opt_parser.value() == 123);
  CHECK(*opt_parser == 123);
  // check cmp
  Optional<int> opt_cmp = std::nullopt;
  CHECK((opt_cmp != Optional(100)) == true);
  // check and_then
  Optional<int> opt = std::nullopt;
  auto opt_and_then = opt.and_then([&](int i) { return i + 1; });
  opt = 42;
  CHECK(opt_and_then == 0);
  auto opt_and_then_2 = opt.and_then([&](int i) -> Optional<int> { return i + 1; });
  CHECK(opt_and_then_2 == 43);
  // check transform
  opt = -42;
  std::unique_ptr<int> up = std::make_unique<int>();
  auto opt_transform = opt.transform([up = std::move(up)](int i) -> int { return i + 1; });
  CHECK(opt_transform == -41);
  // check or_else
  opt = std::nullopt;
  auto opt_or_else = opt.or_else([]() { return 42; });
  CHECK(opt_or_else == 42);
}
