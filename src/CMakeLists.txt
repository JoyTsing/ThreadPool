# track the stack info

# Also requires one of: libbfd (gnu binutils), libdwarf, libdw (elfutils)
FetchContent_Declare(
  backward
  GIT_REPOSITORY https://github.com/bombela/backward-cpp
  GIT_TAG master # or a version tag, such as v1.6
  GIT_SHALLOW TRUE
  SYSTEM # optional, the Backward include directory will be treated as system directory
)
FetchContent_MakeAvailable(backward)

# src
file(GLOB_RECURSE all_src CONFIGURE_DEPENDS *.cpp)

add_executable(main ${all_src})

target_include_directories(main PUBLIC ${PROJECT_SOURCE_DIR}/include/)
target_link_libraries(main PRIVATE Backward::Backward)