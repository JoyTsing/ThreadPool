#pragma once

#include "function/function.h"
class Thread {
  public:
  using Task = Function<void(int)>;  // 任务类型
  Thread(Task func);
  ~Thread() = default;
  void start();
  int getID() const;

  private:
  static int generatedId_;

  private:
  Task func_;
  int threadID_;
};