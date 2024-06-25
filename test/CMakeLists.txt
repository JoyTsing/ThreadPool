find_package(Boost REQUIRED)

file(GLOB_RECURSE all_tests *.cpp)
file(GLOB_RECURSE all_src CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cpp)

foreach(v ${all_tests})
  string(REGEX MATCH "test/.*" relative_path ${v})
  string(REGEX REPLACE "test/" "" target_name ${relative_path})
  string(REGEX REPLACE ".cpp" "" target_name ${target_name})

  add_executable(${target_name} ${v} ${all_src})
  target_link_libraries(${target_name} PUBLIC doctest nanobench
    Backward::Backward Backward::Object)

  if(${target_name} STREQUAL "threadpool_test")
    target_link_libraries(${target_name} PUBLIC ${Boost_LIBRARY_DIRS})
  endif()

  if(${target_name} STREQUAL "memoryhook_test")
    target_link_libraries(${target_name} PRIVATE memoryhook)
  endif()
endforeach()