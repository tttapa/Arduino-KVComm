# KVComm Build and Parse Example

This example demonstrates how to build and parse KVComm dictionaries on your
computer.  
For Arduino examples, see [this page](https://tttapa.github.io/Arduino-KVComm/Doxygen/examples.html).

The code for the example is in [`example.cpp`](example.cpp). The rules for 
compiling it are in [`CMakeLists.txt`](CMakeLists.txt).

## Building and Running the Example

First, install the KVComm library. ==TODO== add link.

Then open a terminal in the `build` folder, run CMake and then Make to configure
and compile the example:

```sh
cmake ..
make -j$(nproc)
```

To run the example program, use:

```sh
./example
```