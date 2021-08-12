cmake_minimum_required(VERSION 3.10)

project(zarks VERSION 0.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-Werror -Wall -Wpedantic)

include_directories(include)

add_subdirectory(src)
add_subdirectory(test)