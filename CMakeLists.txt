cmake_minimum_required(VERSION 3.10)

project(zarks VERSION 0.0)

# C++ options
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_compile_options(-Werror -Wall -Wextra -Wno-unused-parameter -Wpedantic)
IF (CMAKE_BUILD_TYPE MATCHES DEBUG)
    add_compile_definitions(ZARKS_DEBUG)
ENDIF(CMAKE_BUILD_TYPE MATCHES DEBUG)

# Name of the library to be created
set(ZARKS_LIB_NAME ZarkLib)

# All includes should go here
include_directories(include)

# Library source code
add_subdirectory(src)
# Testing
add_subdirectory(test)
# Cool example scripts
add_subdirectory(demo)
