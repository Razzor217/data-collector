# CMake configuration for demo project
cmake_minimum_required(VERSION 3.16)

include_directories(.)

set(CMAKE_CXX_FLAGS "-std=c++17 -msse4.2 -Wall -Wextra -O3 -g")

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

add_executable(event_prototype event_prototype.cpp)
add_executable(regex_prototype regex_prototype.cpp)
