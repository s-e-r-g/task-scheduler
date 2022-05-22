cmake_minimum_required(VERSION 3.5)

project(TaskScheduler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(test_scheduler test.cpp TaskScheduler.cpp)

