#pragma once

// test

int sum1(int a, int b);

int sum2(int a, int b, int c);

// io线程
void io_thread(int listenfd);

// worker线程
void worker_thread(int clientfd);

int expensive_task(int a, int b);

void timeout_task();

void fun1(int slp);