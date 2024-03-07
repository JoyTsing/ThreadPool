#!/bin/bash

# 格式化
clang-format-17 -i $(find include/ -type f -name "*.h")
clang-format-17 -i $(find src/ -type f -name "*.cpp")

# 统计代码行数
cloc --git `git branch --show-current`
