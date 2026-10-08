cmake_minimum_required(VERSION 3.10)
project(hw1_ppd_cpp)

set(CMAKE_CXX_STANDARD 20)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(hw1_ppd_cpp main.cpp)
