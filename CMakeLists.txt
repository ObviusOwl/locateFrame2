cmake_minimum_required(VERSION 3.1)

project (locateFrame2 C CXX)

set (CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory (src) 
# diable tests
#add_subdirectory (test)
enable_testing ()
