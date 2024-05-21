## C++ 20 ThreadPool

基于C++20标准的线程池，需要编译器支持std20标准(用到了20的库)。第三方库使用了doctest和nanobench进行测试，使用backward-cpp进行错误分析和追踪，此外还与boost库进行了对比测试。
## Getting the Source

> git clone --recursive https://github.com/JoyTsing/thread-pool

## Building

本项目基于CMake，在Ubuntu环境下开发

## Build for POSIX
Quick start:

> ./build.sh

## Features

* 实现 C++20 `std::source_location`和`std::fmt`的head-only日志库
* 实现 `std::function`
* 实现可更换等待策略的无锁队列
* 可拓展的线程池本体
* 通过perf分析性能
* ...

## Bench

一个简单的 bench:

![bench](img/bench.png)