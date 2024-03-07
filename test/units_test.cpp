#include <doctest/doctest.h>
#include <nanobench.h>

#include <chrono>
#include <random>
#include <thread>

// NOLINTNEXTLINE
TEST_CASE("tutorial_fast_v1") {
    uint64_t x = 1;
    ankerl::nanobench::Bench().run("++x", [&]() { ++x; });
}

// NOLINTNEXTLINE
TEST_CASE("tutorial_fast_v2") {
    uint64_t x = 1;
    ankerl::nanobench::Bench().run(
        "++x", [&]() { ankerl::nanobench::doNotOptimizeAway(x += 1); });
}

// NOLINTNEXTLINE
TEST_CASE("tutorial_slow_v1") {
    ankerl::nanobench::Bench().run("sleep 100ms, auto", [&] {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });
}

// NOLINTNEXTLINE
TEST_CASE("tutorial_slow_v2") {
    ankerl::nanobench::Bench().epochs(3).run("sleep 100ms", [&] {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });
}

// NOLINTNEXTLINE
TEST_CASE("tutorial_fluctuating_v1") {
    std::random_device dev;
    std::mt19937_64 rng(dev());
    ankerl::nanobench::Bench().run("random fluctuations", [&] {
        // each run, perform a random number of rng calls
        auto iterations = rng() & UINT64_C(0xff);
        for (uint64_t i = 0; i < iterations; ++i) {
            ankerl::nanobench::doNotOptimizeAway(rng());
        }
    });
}
