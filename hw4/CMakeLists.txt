cmake_minimum_required(VERSION 4.3)
project(hw4)

find_package(MPI REQUIRED COMPONENTS CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(hw4 main.cpp
        solutions/AsynchronousCommunication.cpp
        solutions/AsynchronousCommunication.h
        solutions/Extra.cpp
        solutions/Extra.h
        solutions/Sequential.cpp
        solutions/Sequential.h
        solutions/ScatterGather.cpp
        solutions/ScatterGather.h
        solutions/StandardCommunication.cpp
        solutions/StandardCommunication.h
        utils/number_reader.cpp
        utils/number_reader.h
        utils/number_writer.cpp
        utils/number_writer.h)

target_link_libraries(hw4 PRIVATE MPI::MPI_CXX)
