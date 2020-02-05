#pragma once

#include <stdint.h>  // uint#_t
#include <string.h>  // memcpy

/// @addtogroup KVComm
/// @{

/**
 * @file  
 * @brief   KV_Type type definitions for fundamental types (int#_t, uint#_t, 
 *          float, double, bool, char). These definitions specify how variables
 *          of these types should be serialized and deserialized when writing
 *          and reading them from/to the buffer.
 */

/**
 * @brief   Template struct for making types serializable.
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
 *          The type to make serializable.
 */
template <class T>
struct KV_Type {
    static uint8_t getTypeID();
    static size_t getLength();
    static void writeToBuffer(const T &, uint8_t *buffer);
    static T readFromBuffer(const uint8_t *buffer);
};

/// @}

/// Add a KV_Type definition that can be (de)serialized by just `memcpy`ing.
#define KV_ADD_TRIVIAL_TYPE(type, typeid)                                      \
    template <>                                                                \
    struct KV_Type<type> {                                                     \
        inline static uint8_t getTypeID() { return typeid; }                   \
        inline static size_t getLength() { return sizeof(type); }              \
        inline static void writeToBuffer(const type &t, uint8_t *buffer) {     \
            memcpy(buffer, &t, getLength());                                   \
        }                                                                      \
        inline static void readFromBuffer(type &t, const uint8_t *buffer) {    \
            memcpy(&t, buffer, getLength());                                   \
        }                                                                      \
    }

KV_ADD_TRIVIAL_TYPE(int8_t, 1);
KV_ADD_TRIVIAL_TYPE(uint8_t, 2);
KV_ADD_TRIVIAL_TYPE(int16_t, 3);
KV_ADD_TRIVIAL_TYPE(uint16_t, 4);
KV_ADD_TRIVIAL_TYPE(int32_t, 5);
KV_ADD_TRIVIAL_TYPE(uint32_t, 6);
KV_ADD_TRIVIAL_TYPE(int64_t, 7);
KV_ADD_TRIVIAL_TYPE(uint64_t, 8);
KV_ADD_TRIVIAL_TYPE(float, 9);
KV_ADD_TRIVIAL_TYPE(double, sizeof(double) == sizeof(float) ? 9 : 10);
KV_ADD_TRIVIAL_TYPE(bool, 11);
KV_ADD_TRIVIAL_TYPE(char, 12);

#if defined(__arm__)
// TODO: integers are a separate data type on ARM
// are there more platforms with this quirk?
KV_ADD_TRIVIAL_TYPE(int, 5);
KV_ADD_TRIVIAL_TYPE(unsigned int, 6);
#endif