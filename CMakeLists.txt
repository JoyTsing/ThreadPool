cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
  message("Setting default build type to Debug")
endif()

project(thread-pool VERSION 0.0.1 LANGUAGES CXX)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_subdirectory(third-party/)
add_subdirectory(examples/)
add_subdirectory(utils/)
add_subdirectory(test/)