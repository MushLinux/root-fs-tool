cmake_minimum_required(VERSION 4.3)
project(root-fs-tool)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES src/main.cpp)

add_executable(root-fs-tool ${SOURCES})