## threadpool

基于C++20的线程池，需要编译器支持std20标准(用到了20的库)，第三方库使用了doctest和nanobench进行测试，使用backward-cpp进行错误分析和追踪，此外还与boost库进行了对比测试：

* 实现 C++20 `std::source_location`和`std::fmt`的head-only日志库
* 实现 `std::function`
* 实现可更换等待策略的无锁队列
* 可拓展的线程池本体
* 通过perf分析性能
* ...

![bench](img/bench.png)