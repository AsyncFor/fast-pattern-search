cmake_minimum_required(VERSION 3.9)

project(fast_pattern_search)

set(CMAKE_CXX_STANDARD 17)


file(GLOB SOURCE_FILES src/*.cpp)
add_executable(fast_pattern_search ${SOURCE_FILES})
target_include_directories(fast_pattern_search PUBLIC include)
