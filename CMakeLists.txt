# CMake configuration for demo project
cmake_minimum_required(VERSION 3.16)

project(data-collector)

include_directories(.)

set(CMAKE_CXX_FLAGS "-std=c++17 -msse4.2 -Wall -Wextra -O3 -g")

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

set(HEADERS
    collector.h
    fifo.h
)

set(SOURCES
    collector.cpp
)

add_executable(event_prototype event_prototype.cpp)
add_executable(regex_prototype regex_prototype.cpp)

add_executable(collector main.cpp ${HEADERS} ${SOURCES})
target_link_libraries(collector Threads::Threads stdc++fs)


set(GTEST_ROOT /usr/src/googletest)

find_path(GTEST_ROOT googletest/CMakeLists.txt HINTS ${GTEST_ROOT})

if (NOT ${GTEST_ROOT} STREQUAL GTEST_ROOT-NOTFOUND)
    enable_testing()

    add_subdirectory(${GTEST_ROOT} ${CMAKE_BINARY_DIR}/googletest EXCLUDE_FROM_ALL)

    add_executable(correctness_test test/correctness.cpp ${HEADERS} ${SOURCES})
    add_executable(component_test test/component.cpp ${HEADERS} ${SOURCES})
    target_link_libraries(correctness_test gtest_main stdc++fs)
    target_link_libraries(component_test gtest_main Threads::Threads stdc++fs)

    include(GoogleTest)
    gtest_discover_tests(correctness_test)
    gtest_discover_tests(component_test)
endif()

