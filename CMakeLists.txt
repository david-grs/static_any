cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_library(dyn_lib SHARED dyn_lib.cpp dyn_lib.hpp)

add_executable(tests any.hpp unit_tests.cpp)
target_link_libraries(tests dyn_lib ${GTEST_BOTH_LIBRARIES} pthread)

set(BUILD_BENCHMARK OFF CACHE BOOL "Build benchmark against boost.any and qvariant")
if(BUILD_BENCHMARK)
    if(NOT CMAKE_BUILD_TYPE MATCHES Release)
      message(WARNING "Benchmark should be build in Release mode")
    endif()

    find_package(Qt4 REQUIRED)
    add_executable(benchmark benchmark.cpp)
    target_link_libraries(benchmark papi Qt4::QtCore)
endif()
