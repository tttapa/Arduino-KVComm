cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 14)
project(kvcomm-build-and-parse-example)

find_package(KVComm REQUIRED)

add_executable(example example.cpp)
target_link_libraries(example PUBLIC kvcomm)