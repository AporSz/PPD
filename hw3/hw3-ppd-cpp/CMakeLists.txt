cmake_minimum_required(VERSION 3.20)
project(hw3_ppd_cpp)

set(CMAKE_CXX_STANDARD 20)

add_executable(hw3_ppd_cpp
        main.cpp
        model/Matrix.cpp
        model/Matrix.h
        model/DynamicMatrix.h
        model/DynamicMatrix.cpp)
