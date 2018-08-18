cmake_minimum_required(VERSION 3.11)
project(workq++)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED 11)

option(WORKQ_DEBUG "enable debugging output" off)

add_subdirectory(vendor)
include_directories(vendor/c-thread-pool)
add_subdirectory(src)
add_subdirectory(example)