include(gtest.cmake)

add_executable(tests unit_tests.cpp)
add_library(dyn_lib SHARED dyn_lib.cpp dyn_lib.hpp)

find_package (Threads)
target_link_libraries(tests PRIVATE dyn_lib gtest ${CMAKE_THREAD_LIBS_INIT})

if (MSVC)
	set(cxx_compile_options /std:c++14 /W4 /WX)

	# VS 2017 removed tr1
	add_definitions(-DGTEST_HAS_TR1_TUPLE=0)
else()
	set(cxx_compile_options -std=c++14 -g -Wall -Wextra -Wpedantic -Wconversion -Wswitch-default -Wswitch-enum -Wunreachable-code -Wwrite-strings -Wcast-align -Wshadow -Wundef -Wno-switch-default -Wold-style-cast -Wshadow -Wdouble-promotion)

	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		set(cxx_compile_options ${cxx_compile_options} -Weverything -Wno-c++98-compat -Wno-global-constructors)
	elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		set(cxx_compile_options ${cxx_compile_options} -Wuseless-cast -Wduplicated-cond -Wlogical-op)
	endif()

	if (COVERAGE)
		target_compile_options(tests PRIVATE --coverage)
		target_link_libraries(tests PRIVATE --coverage)
	endif()
endif()

target_compile_options(tests PRIVATE ${cxx_compile_options})
target_compile_options(dyn_lib PRIVATE ${cxx_compile_options})
