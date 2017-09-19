cmake_minimum_required(VERSION 2.8)

project(static_any)
include(gtest.cmake)

SET(COVERAGE OFF CACHE BOOL "Coverage")
set(BUILD_BENCHMARK OFF CACHE BOOL "Build benchmark against boost.any and qvariant")

add_executable(tests any.hpp unit_tests.cpp)

add_library(dyn_lib SHARED dyn_lib.cpp dyn_lib.hpp)

find_package (Threads)
target_link_libraries(tests dyn_lib gtest ${CMAKE_THREAD_LIBS_INIT})

if (MSVC)
	target_compile_options(tests PRIVATE /std:c++17 /W4 /WX)

	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ox")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od")

	# VS 2017 removed tr1
	add_definitions(-DGTEST_HAS_TR1_TUPLE=0)
else()
	target_compile_options(tests PRIVATE -std=c++14 -g -Wall -Wextra -Wpedantic -Wconversion -Wswitch-default -Wswitch-enum -Wunreachable-code -Wwrite-strings -Wcast-align -Wshadow -Wundef -Wno-switch-default)

	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		target_compile_options(tests PRIVATE -Weverything -Wno-c++98-compat -Wno-global-constructors)
	endif()

	if (COVERAGE)
		target_compile_options(tests PRIVATE --coverage)
		target_link_libraries(tests PRIVATE --coverage)
	endif()
endif()

if(BUILD_BENCHMARK)
	if(NOT CMAKE_BUILD_TYPE MATCHES Release)
		message(WARNING "Benchmark should be build in Release mode")
	endif()

	find_package(Qt4 REQUIRED)
	add_executable(benchmark benchmark.cpp)
	target_link_libraries(benchmark papi Qt4::QtCore)
endif()

