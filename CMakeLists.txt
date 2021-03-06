cmake_minimum_required(VERSION 3.17)
project(hw3)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Wall -pedantic)

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

add_custom_target(
        flex
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND flex ${CMAKE_CURRENT_SOURCE_DIR}/scanner.lex
)

add_custom_target(
        bison
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND bison -d ${CMAKE_CURRENT_SOURCE_DIR}/parser.ypp
)

add_dependencies(hw3 flex)

add_dependencies(hw3 bison)

# compile lex.yy.c as c++
set_source_files_properties(lex.yy.c PROPERTIES LANGUAGE CXX )
