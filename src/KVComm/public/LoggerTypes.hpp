#pragma once

#include <stdint.h> // uint#_t
#include <string.h> // memcpy

/// @addtogroup logger
/// @{

/**
 * @file  
 * @brief   Logger type definitions for fundamental types (int#_t, uint#_t, 
 *          float, double, bool, char) to make them loggable.
 */

/**
 * @brief   Template struct for making types loggable.
 * 
 * Specializations should implement the following methods:
 *  - `static uint8_t getTypeID()`
 *  - `static size_t getLength()`
 *  - `static void writeToBuffer(const T &, uint8_t *buffer)`
 *  - `static T readFromBuffer(const uint8_t *buffer)`
 * 
 * `getTypeID` should return an arbitrary, unique identifier for the type.  
 * `getLength` returns the size of the type in bytes.  
 * `writeToBuffer` copies an object of the type into a byte buffer (it 
 * shouldn't write more than `getLength()` bytes).  
 * `readFromBuffer` converts a buffer of bytes into an object of the type (it
 * shouldn't read more than `getLength()` bytes).
 * 
 * When copying data from and to the buffer, don't break the 
 * [strict aliasing rule](https://en.wikipedia.org/wiki/Pointer_aliasing)
 * and keep in mind that the compiler can add 
 * [padding bytes](https://en.wikipedia.org/wiki/Data_structure_alignment) to 
 * structs.
 * 
 * @tparam  T
 *          The type to make loggable.
 */
template <class T>
struct LoggableType {
    // static uint8_t getTypeID();
    // static size_t getLength();
    // static void writeToBuffer(const T &, uint8_t *buffer);
    // static T readFromBuffer(const uint8_t *buffer);
};

/// Add a type to the logger that can be logged by just `memcpy`ing.
#define LOGGER_ADD_TRIVIAL_TYPE(type, typeid)                                  \
    template <>                                                                \
    struct LoggableType<type> {                                                \
        inline static uint8_t getTypeID() { return typeid; }                   \
        inline static size_t getLength() { return sizeof(type); }              \
        inline static void writeToBuffer(const type &t, uint8_t *buffer) {     \
            memcpy(buffer, &t, getLength());                                   \
        }                                                                      \
        inline static type readFromBuffer(const uint8_t *buffer) {             \
            type t;                                                            \
            memcpy(&t, buffer, getLength());                                   \
            return t;                                                          \
        }                                                                      \
    }

LOGGER_ADD_TRIVIAL_TYPE(int8_t, 1);
LOGGER_ADD_TRIVIAL_TYPE(uint8_t, 2);
LOGGER_ADD_TRIVIAL_TYPE(int16_t, 3);
LOGGER_ADD_TRIVIAL_TYPE(uint16_t, 4);
LOGGER_ADD_TRIVIAL_TYPE(int32_t, 5);
LOGGER_ADD_TRIVIAL_TYPE(uint32_t, 6);
LOGGER_ADD_TRIVIAL_TYPE(int64_t, 7);
LOGGER_ADD_TRIVIAL_TYPE(uint64_t, 8);
LOGGER_ADD_TRIVIAL_TYPE(float, 9);
LOGGER_ADD_TRIVIAL_TYPE(double, 10);
LOGGER_ADD_TRIVIAL_TYPE(bool, 11);
LOGGER_ADD_TRIVIAL_TYPE(char, 12);

// int and int32_t are different types on Zybo.
#ifndef __x86_64__
LOGGER_ADD_TRIVIAL_TYPE(int, 5);
LOGGER_ADD_TRIVIAL_TYPE(unsigned int, 6);
#endif

/// @}