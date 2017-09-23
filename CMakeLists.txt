cmake_minimum_required(VERSION 2.8)

project(static_any)

set(COVERAGE OFF CACHE BOOL "Coverage")
set(BUILD_BENCHMARK OFF CACHE BOOL "Build benchmark against boost.any and qvariant")

add_subdirectory(tests)

if(BUILD_BENCHMARK)
    add_subdirectory(benchmark)
endif()

