cmake_minimum_required(VERSION 3.19)
project(hw3)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(hw3
        hw3_output.cpp
        hw3_output.hpp
        Scope.cpp
        Scope.h)
