[![Build Status](https://github.com/tttapa/Arduino-KVComm/workflows/CI%20Tests/badge.svg)](https://github.com/tttapa/Arduino-KVComm/actions)
[![Test Coverage](https://img.shields.io/endpoint?url=https://raw.githubusercontent.com/tttapa/Arduino-KVComm/master/docs/Coverage/shield.io.coverage.json)](https://tttapa.github.io/Arduino-KVComm/Coverage/index.html)
[![Build Status](https://travis-ci.org/tttapa/Arduino-KVComm.svg?branch=master)](https://travis-ci.org/tttapa/Arduino-KVComm)
[![GitHub](https://img.shields.io/github/stars/tttapa/Arduino-KVComm?label=GitHub&logo=github)](https://github.com/tttapa/Arduino-KVComm)

# Arduino KVComm

This is a library to write key-value data into a dictionary-like data structure,
in order to communicate between Arduinos, or between an Arduino and a computer.

The library can be used in the Arduino IDE and as a C++ library for your 
computer.

## Documentation

The automatically generated Doxygen documentation for this library can be found 
here:  
[**Documentation**](https://tttapa.github.io/Arduino-KVComm/Doxygen/index.html)  
Test coverage information can be found here:  
[**Code Coverage**](https://tttapa.github.io/Arduino-KVComm/Coverage/index.html)  
Arduino examples can be found here:  
[**Examples**](https://tttapa.github.io/Arduino-KVComm/Doxygen/examples.html)

It's a good idea to start with the documentation in the 
[**KVComm module**](https://tttapa.github.io/Arduino-KVComm/Doxygen/d4/d09/group__KVComm.html).

## Supported boards

For each commit, the continuous integration tests compile the examples for the
following boards:

- Arduino UNO
- Arduino Leonardo
- Teensy 3.2
- Arduino Due
- Arduino Nano 33 IoT
- ESP8266
- ESP32

This covers a very large part of the Arduino platform, and similar boards will
also work (e.g. Arduino Nano, Arduino Mega, etc.).

If you have a board that's not supported, please 
[open an issue](https://github.com/tttapa/Arduino-KVComm/issues/new)
and let me know!