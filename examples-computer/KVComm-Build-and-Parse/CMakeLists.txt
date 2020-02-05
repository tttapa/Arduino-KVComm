cmake_minimum_required(VERSION 3.10)
project(kvcomm-build-and-parse-example)

# Find the KVComm library (must be installed first)
find_package(KVComm REQUIRED)

# Add a target that compiles the example program
add_executable(example example.cpp)
# Link the KVComm library to the example program
target_link_libraries(example PUBLIC kvcomm)