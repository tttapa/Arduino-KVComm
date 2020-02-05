cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 14)
project(KVComm)

add_library(kvcomm
    src/KV_Iterator.cpp
    src/KV_Builder.cpp
    src/KV_Parser.cpp
)
target_include_directories(kvcomm 
    PUBLIC
        include
    PRIVATE
        src
)

add_library(kvcomm_arduino EXCLUDE_FROM_ALL
    src/KV_Iterator.cpp
    src/KV_Builder.cpp
    src/KV_Parser.cpp
)
target_include_directories(kvcomm_arduino 
    PUBLIC
        include
    PRIVATE
        src
)
target_compile_definitions(kvcomm_arduino PUBLIC ARDUINO_TEST)
target_link_libraries(kvcomm_arduino PRIVATE Arduino_Helpers)
