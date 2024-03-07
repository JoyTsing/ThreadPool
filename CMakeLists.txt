cmake_minimum_required(VERSION 3.20)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

project(main LANGUAGES CXX)

include(FetchContent)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_subdirectory(src/)
add_subdirectory(test/)