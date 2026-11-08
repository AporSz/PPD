cmake_minimum_required(VERSION 4.3)
project(hw4)

set(CMAKE_CXX_STANDARD 20)

add_executable(hw4 main.cpp
        solutions/Sequential.cpp
        solutions/Sequential.h
        utils/number_reader.cpp
        utils/number_reader.h
        utils/number_writer.cpp
        utils/number_writer.h)
