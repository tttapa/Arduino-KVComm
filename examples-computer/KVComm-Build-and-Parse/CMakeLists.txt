cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 14)
project(kvcomm-build-and-parse)

add_subdirectory(../../src/KVComm KVComm)

add_executable(kvcomm-build-and-parse main.cpp)
target_link_libraries(kvcomm-build-and-parse PUBLIC kvcomm)