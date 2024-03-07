cmake_minimum_required(VERSION 3.20)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(main LANGUAGES CXX)

include(FetchContent)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_subdirectory(src/)
add_subdirectory(test/)