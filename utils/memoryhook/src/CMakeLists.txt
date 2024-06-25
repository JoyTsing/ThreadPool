file(GLOB_RECURSE all_examples *.cpp)
add_library(memoryhook SHARED ${all_examples})