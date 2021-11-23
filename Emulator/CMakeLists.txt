cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

# Set project name
project(vAmiga)

# Add include paths
include_directories(${CMAKE_SOURCE_DIR}/..)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# List all source files
set(files Dummy.cpp)
add_executable(vAmiga ${files})

# Add sub directories
add_subdirectory(Utilities)

target_link_libraries(vAmiga Utilities)
