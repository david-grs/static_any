if(NOT CMAKE_BUILD_TYPE MATCHES Release)
    message(WARNING "Benchmark should be build in Release mode")
endif()

find_package(Qt4 REQUIRED)
add_executable(benchmark benchmark.cpp)
target_link_libraries(benchmark papi Qt4::QtCore)

