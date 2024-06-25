# for each "example/x.cpp", generate target "x"
file(GLOB_RECURSE all_examples *.cpp)

foreach(v ${all_examples})
  string(REGEX MATCH "examples/.*" relative_path ${v})

  string(REGEX REPLACE "examples/" "" target_name ${relative_path})
  string(REGEX REPLACE ".cpp" "" target_name ${target_name})

  add_executable(${target_name} ${v})
  target_link_libraries(${target_name} PRIVATE memoryhook)
endforeach()
