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
