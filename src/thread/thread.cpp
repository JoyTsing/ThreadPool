#include "thread/thread.h"
#include <thread>

int Thread::generatedId_ = 0;

Thread::Thread(Task func)
    : func_(std::move(func)),
      threadID_(generatedId_++) {}

void Thread::start() {
  std::thread t(func_, threadID_);
  t.detach();
}

int Thread::getID() const { return threadID_; }