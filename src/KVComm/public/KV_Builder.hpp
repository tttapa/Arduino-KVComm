#pragma once

#include <KVComm/private/KV_Iterator.hpp>  // KV_Iterator

#include <AH/STL/array>             // std::array
#include <AH/STL/cstddef>           // size_t
#include <AH/STL/initializer_list>  // std::initializer_list
#include <AH/STL/vector>            // std::vector
#include <string.h>                 // strlen

#ifndef ARDUINO
#include <iosfwd>  // std::ostream forward declaration
#endif

/**
 * @file  
 * @brief   This file contains the KV_Builder class, a key-value pair, 
 *          dictionary-like container that supports many different types of data
 *          (numbers, arrays, vectors, strings ...).
 */

/**
 * @brief   A class for serializing key-value/array data in a dictionary-like
 *          container that can be sent to another Arduino or to a computer.
 * 
 * The KV_Builder class provides an easy to use KV_Builder::add method that can 
 * be used with single values/objects, strings, arrays of values/objects, and 
 * vectors of values/objects.
 * 
 * ~~~cpp
 * Static_KV_Builder<256> dict;
 * int values[] = {1, 2, 3};
 * dict.add("key", values);
 * ~~~
 * 
 * User types can be added to the dictionary as well, by specializing the
 * @ref KV_Type template.
 * 
 * **Data structure**
 * 
 * The dictionary is just a long array of bytes. Each key-value pair in the 
 * dictionary starts with a 4-byte header that contains a unique identifier of 
 * the type of the data, the length of the key (in bytes) and the length of the 
 * data (in bytes). 
 *     
 *          0        1        2        3
 *     +---------+---------+---------+---------+
 *     | key len |  type   |    data length    |
 *     +---------+---------+---------+---------+
 *     |                  key                  |
 *     +---------+---------+---------+---------+
 *     |                  ...                  |
 *     +---------+---------+---------+---------+
 *     |             key             |  NULL   |
 *     +---------+---------+---------+---------+
 *     |                 data                  |
 *     +---------+---------+---------+---------+
 *     |                  ...                  |
 *     +---------+---------+---------+---------+
 *     
 *     └─1 byte──┘ 
 *     └────────────────1 word─────────────────┘
 * 
 * The header is always aligned to a word boundary.
 * The key starts at the second word.
 * There is at least one NULL byte after the key, and at most four.
 * The data always starts at a word boundary,
 * right after the null bytes after the key. The data can be a single 
 * object, or an array of objects, without any padding.
 * 
 * The type identifier is defined using the KV_Type structs. These structs
 * also define how objects of that type should be copied into or from the 
 * buffer, as well as the size of a single object.
 * 
 * There are two main types of data: fundamental types (int, float, bool, etc.)
 * that can be `memcpy`d into or from the buffer, and user-defined types that 
 * have special functions for copying their data.  
 * The former are defined in @ref KV_Types.hpp, and an example of the latter 
 * can be found in @ref LoggerMathTypes.hpp.
 * @todo LoggerMathTypes.hpp
 * 
 * Structured data cannot be `memcpy`d, because the compiler doesn't necessarily
 * pack all of the data together, it's free to allow padding bytes where 
 * necessary. 
 * ([Wikipedia: Data structure alignment](https://en.wikipedia.org/wiki/Data_structure_alignment))
 * 
 * String data is copied into the dictionary as an array of characters. The null
 * terminator is included.
 */
class KV_Builder {
  public:
    KV_Builder(uint8_t *buffer, size_t bufferSize)
        : buffer(buffer), bufferSize(bufferSize) {}

    /**
     * @brief   Add a key-value pair to the dictionary, or update the existing
     *          value with the same key.
     * 
     * If the key can't be found in the dictionary, the new element is 
     * append%ed. If a value with the same key is found, and if the type and
     * size are the same, the existing value is updated with the new value. If 
     * the type or size don't match, the dictionary is not altered.
     * 
     * @tparam  T 
     *          The type of the value to add.
     * @param   key 
     *          The key for this element.
     *          The maximum length is 255 characters.
     * @param   data
     *          A pointer to the (array of) value(s) to add.
     * @param   count
     *          The number of elements in the array.
     *          The maximum size is 65535 bytes, so the maximum count is 65535
     *          divided by the size of one element.
     * @retval  true
     *          The element was added to the dictionary successfully, or the 
     *          existing element with the same key was updated with the new
     *          value. 
     * @retval  false 
     *          The buffer is full, the key or data length is too large, 
     *          or the type and size don't match the ones of the existing 
     *          element with the same key.
     */
    template <class T>
    bool add(const char *key, const T *data, size_t count) {
        // don't allow empty keys
        if (key == nullptr || key[0] == '\0')
            return false;
        // don't allow null data
        if (data == nullptr && count != 0)
            return false;
        // Check if dictionary already contains a value with this key
        KV_Iterator::iterator found = find(key);
        if (found != KV_Iterator::end())
            // if the dictionary already contains an element with the same key
            return overwrite(found, data, count);
        else
            // this is a new key
            return append(key, data, count);
    }

    /**
     * @brief   Add a key-value pair to the dictionary, or update the existing
     *          value with the same key. The data of the element is a single 
     *          value.
     * 
     * If the key can't be found in the dictionary, the new element is 
     * append%ed. If a value with the same key is found, and if the type and
     * size are the same, the existing value is updated with the new value. If 
     * the type or size don't match, the dictionary is not altered.
     * 
     * @tparam  T
     *          The type of the value to add.
     * @param   key 
     *          The key for this element.
     *          The maximum length is 255 characters.
     * @param   value
     *          The value to add.
     *          The maximum size is 65535 bytes.
     * @retval  true
     *          The element was added to the dictionary successfully, or the 
     *          existing element with the same key was updated with the new
     *          value. 
     * @retval  false 
     *          The buffer is full, the key or data length is too large, 
     *          or the type and size don't match the ones of the existing 
     *          element with the same key.
     */
    template <class T>
    bool add(const char *key, const T &value) {
        return add(key, &value, 1);
    }

    /**
     * @brief   Add a key-value pair to the dictionary, or update the existing
     *          value with the same key. The data of the element is a 
     *          null-terminated string.
     * 
     * If the key can't be found in the dictionary, the new element is 
     * append%ed. If a value with the same key is found, and if the type and
     * size are the same, the existing value is updated with the new value. If 
     * the type or size don't match, the dictionary is not altered.
     * 
     * @param   key 
     *          The key for this element.
     *          The maximum length is 255 characters.
     * @param   data 
     *          The string to add.
     *          The maximum length is 65534 characters.
     * @retval  true
     *          The element was added to the dictionary successfully, or the 
     *          existing element with the same key was updated with the new
     *          value. 
     * @retval  false 
     *          The buffer is full, the key or data length is too large, 
     *          or the type and size don't match the ones of the existing 
     *          element with the same key.
     */
    bool add(const char *key, const char *data) {
        return add(key, data, strlen(data) + 1);
    }

    /**
     * @brief   Add a key-value pair to the dictionary, or update the existing
     *          value with the same key. The data of the element is an array 
     *          of values.
     * 
     * If the key can't be found in the dictionary, the new element is 
     * append%ed. If a value with the same key is found, and if the type and
     * size are the same, the existing value is updated with the new value. If 
     * the type or size don't match, the dictionary is not altered.
     * 
     * @tparam  T
     *          The type of the values to add.
     * @tparam  N
     *          The number of values to add.
     * @param   key 
     *          The key for this element.
     *          The maximum length is 255 characters.
     * @param   array 
     *          The array of values to add.
     *          The maximum size is 65535 bytes.
     * @retval  true
     *          The element was added to the dictionary successfully, or the 
     *          existing element with the same key was updated with the new
     *          value. 
     * @retval  false 
     *          The buffer is full, the key or data length is too large, 
     *          or the type and size don't match the ones of the existing 
     *          element with the same key.
     */
    template <class T, size_t N>
    bool add(const char *key, const T (&array)[N]) {
        return add(key, array, N);
    }

    /**
     * @brief   Add a key-value pair to the dictionary, or update the existing
     *          value with the same key. The data of the element is a list 
     *          of values.
     * 
     * If the key can't be found in the dictionary, the new element is 
     * append%ed. If a value with the same key is found, and if the type and
     * size are the same, the existing value is updated with the new value. If 
     * the type or size don't match, the dictionary is not altered.
     * 
     * @tparam  T
     *          The type of the values to add.
     * @param   key 
     *          The key for this element.
     *          The maximum length is 255 characters.
     * @param   list 
     *          The list of values to add.
     *          The maximum size is 65535 bytes.
     * @retval  true
     *          The element was added to the dictionary successfully, or the 
     *          existing element with the same key was updated with the new
     *          value. 
     * @retval  false 
     *          The buffer is full, the key or data length is too large, 
     *          or the type and size don't match the ones of the existing 
     *          element with the same key.
     */
    template <class T>
    bool add(const char *key, std::initializer_list<T> list) {
        return add(key, list.begin(), list.size());
    }

    /**
     * @brief   Add a key-value pair to the dictionary, or update the existing
     *          value with the same key. The data of the element is an 
     *          std::array of values.
     * 
     * If the key can't be found in the dictionary, the new element is 
     * append%ed. If a value with the same key is found, and if the type and
     * size are the same, the existing value is updated with the new value. If 
     * the type or size don't match, the dictionary is not altered.
     * 
     * @tparam  T
     *          The type of the values to add.
     * @tparam  N
     *          The number of values to add.
     * @param   key 
     *          The key for this element.
     *          The maximum length is 255 characters.
     * @param   array 
     *          The array of values to add.
     *          The maximum size is 65535 bytes.
     * @retval  true
     *          The element was added to the dictionary successfully, or the 
     *          existing element with the same key was updated with the new
     *          value. 
     * @retval  false 
     *          The buffer is full, the key or data length is too large, 
     *          or the type and size don't match the ones of the existing 
     *          element with the same key.
     */
    template <class T, size_t N>
    bool add(const char *key, const std::array<T, N> &array) {
        return add(key, array.data(), N);
    }

    /**
     * @brief   Add a key-value pair to the dictionary, or update the existing
     *          value with the same key. The data of the element is an 
     *          std::vector of values.
     * 
     * If the key can't be found in the dictionary, the new element is 
     * append%ed. If a value with the same key is found, and if the type and
     * size are the same, the existing value is updated with the new value. If 
     * the type or size don't match, the dictionary is not altered.
     * 
     * @tparam  T
     *          The type of the values to add.
     * @param   key 
     *          The key for this element.
     *          The maximum length is 255 characters.
     * @param   vector 
     *          The vector of values to add.
     *          The maximum size is 65535 bytes.
     * @retval  true
     *          The element was added to the dictionary successfully, or the 
     *          existing element with the same key was updated with the new
     *          value. 
     * @retval  false 
     *          The buffer is full, the key or data length is too large, 
     *          or the type and size don't match the ones of the existing 
     *          element with the same key.
     */
    template <class T>
    bool add(const char *key, const std::vector<T> &vector) {
        return add(key, vector.data(), vector.size());
    }

    /**
     * @brief   Clear all elements of the dictionary.
     */
    void clear() {
        bufferwritelocation = buffer;
        maxLen              = bufferSize;
        std::fill(buffer, buffer + bufferSize, 0);
    }

#ifndef ARDUINO
    void print(std::ostream &os) const;
    void printPython(std::ostream &os) const;
#endif  // ARDUINO

    /**
     * @brief   Dump the dictionary buffer to the given output stream in a 
     *          human-readable format (offset + hexadecimal + ASCII).
     * 
     * @param   os
     *          The stream to print to.
     */
    void print(Print &os) const;

    /**
     * @brief   Dump the dictionary buffer to the given output stream as a 
     *          Python bytes object.
     * 
     * @param   os
     *          The stream to print to.
     */
    void printPython(Print &os) const;

  private:
    uint8_t *buffer;
    size_t bufferSize;
    /// The maximum remaining length that is still free.
    /// Starts off as the buffer size, reaches zero when the buffer is full.
    size_t maxLen = bufferSize;

    /// A pointer to the first free/unused byte in the buffer.
    uint8_t *bufferwritelocation = buffer;

    /// Write the header of a new element into the buffer, advance the write
    /// pointer, and return a pointer to where the data should be written.
    /// Returns a null pointer if the element is too large for the buffer, and
    /// in that case, the write pointer is unaltered.
    uint8_t *writeHeader(const char *key, uint8_t typeID, size_t length);

    /// Append the new element to the buffer.
    /// Returns false if the element is too large for the buffer.
    template <class T>
    bool append(const char *key, const T *data, size_t count);

    /// Overwrite the existing element referenced by @p existing with the
    /// new data, if the type and size match.
    /// Returns false if the type or size doesn't match, and in that case,
    /// nothing is overwritten.
    template <class T>
    bool overwrite(KV_Iterator::iterator existing, const T *data, size_t count);

    /// (Over)write the data of an element to the buffer.
    template <class T>
    void overwrite(uint8_t *buffer, const T *data, size_t count);

  public:
    /// Get the buffer containing the dictionary.
    const uint8_t *getBuffer() const { return buffer; }
    /// Get the total size of the buffer.
    size_t getBufferSize() const { return bufferSize; }
    /// Get the length of the used part of the buffer.
    size_t getLength() const { return getBufferSize() - maxLen; }

    /** 
     * @brief   Get the element with the given key.
     * 
     * @param   key
     *          The unique key of the element to retreive.
     * @return  An iterator to the element with the given key, or a 
     *          default-constructed KV_Iterator::iterator if the key was 
     *          not found in the dictionary.
     */
    KV_Iterator::iterator find(const char *key) const;
};

/// Macro for easily adding variables with the variable name as the key.
#define ADD_VAR(dict, var) dict.add(#var, var)

/// KV_Builder with a static buffer
template <size_t N>
class Static_KV_Builder : public KV_Builder {
  public:
    Static_KV_Builder() : KV_Builder(buffer.data(), N) {}

  private:
    std::array<uint8_t, N> buffer = {{}};
};

#include <KVComm/src/KV_Builder.ipp>