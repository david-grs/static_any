cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++14 -g -Wall -Wextra")

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(tests any.hpp unit_tests.cpp)
target_link_libraries(tests ${GTEST_BOTH_LIBRARIES} pthread)



