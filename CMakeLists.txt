cmake_minimum_required(VERSION 3.17)
project(hw3)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)

add_executable(hw3
        hw3_output.cpp
        hw3_output.hpp
        Semantics.cpp
        Semantics.h
        scanner.lex
        parser.ypp
        lex.yy.c
        parser.tab.cpp
        parser.tab.hpp)
