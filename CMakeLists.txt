cmake_minimum_required(VERSION 3.14)
project(connect_four)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_FLAGS "-O3")


add_executable(connect_four main.cpp connect-four.cpp connect-four.h)