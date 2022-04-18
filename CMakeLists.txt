cmake_minimum_required(VERSION 3.10)

project(zarks VERSION 0.2)

# Convert CMAKE_BUILD_TYPE to upper case for easy comparison
string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)

# General C++ options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_compile_options(-Werror -Wall -Wextra -Wno-unused-parameter -pedantic)

# Determine logging and optimization levels based on CMAKE_BUILD_TYPE
IF (CMAKE_BUILD_TYPE MATCHES DEBUG)
    add_compile_definitions(ZARKS_DEBUG)
    add_compile_options(-g)
    message("Debug mode!")
ELSEIF (CMAKE_BUILD_TYPE MATCHES RELEASE)
    add_compile_options(-O3)
    message("Release mode!")
ENDIF (CMAKE_BUILD_TYPE MATCHES DEBUG)

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
