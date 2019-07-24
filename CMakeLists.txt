cmake_minimum_required(VERSION 2.8.9)
project(msexecrc)

add_definitions(-Wall -Wextra -Werror -O3)

include_directories("./include")

add_executable(msexecrc src/msexecrc.cpp)
