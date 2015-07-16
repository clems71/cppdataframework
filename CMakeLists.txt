cmake_minimum_required(VERSION 3.0)
project(cppdataframework)

enable_testing()

set(CMAKE_DEBUG_POSTFIX "d")

# Compiler flags (C++14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++1y -g")

# External libraries
add_subdirectory(libs/gtest)
add_subdirectory(libs/jsoncpp)

# Library source code
add_subdirectory(src)

# Server app
add_subdirectory(tests)
