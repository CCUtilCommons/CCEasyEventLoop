cmake_minimum_required(VERSION 3.10)

# C++ Settings for the EventLoops
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE "Debug")
# Projects
project(CCEasyEventLoop VERSION 0.1.0 LANGUAGES CXX)

add_subdirectory(library)
add_subdirectory(test)


