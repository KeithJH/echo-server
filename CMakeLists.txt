cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(EchoServer CXX)

include(cmake/Common.cmake)

add_subdirectory(src)
