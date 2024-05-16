# for each "src/x.cpp", generate target "x"
file(GLOB_RECURSE all_examples *.cpp)
file(GLOB_RECURSE all_srcs CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cpp)

foreach(v ${all_examples})
  string(REGEX MATCH "examples/.*" relative_path ${v})
  string(REGEX REPLACE "examples/" "" target_name ${relative_path})
  string(REGEX REPLACE ".cpp" "" target_name ${target_name})

  add_executable(${target_name} ${v} ${all_srcs})
  target_link_libraries(${target_name} PUBLIC Backward::Backward Backward::Interface Backward::Object)
endforeach()
