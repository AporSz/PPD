cmake_minimum_required(VERSION 4.3)
project(hw2_ppd_cpp)

set(CMAKE_CXX_STANDARD 20)

add_executable(hw2_ppd_cpp main.cpp
        model/Matrix.cpp
        model/Matrix.h)
