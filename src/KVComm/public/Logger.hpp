#pragma once

#include <KVComm/private/LogEntryIterator.hpp>  // LogEntryIterator
#include <KVComm/public/LoggerMathTypes.hpp>    // LoggableType

#include <AH/STL/array>  // std::array
#include <AH/STL/cstddef> // size_t
#include <AH/STL/vector> // std::vector
#include <AH/STL/initializer_list> // std::initializer_list
#include <string.h>      // strlen

#ifndef ARDUINO
#include <iosfwd>  // std::ostream forward declaration
#endif

/// @addtogroup logger
/// @{

/**
 * @file  
 * @brief   This file contains the Logger class, a key-value pair logger that 
 *          supports many different types of logging data (numbers, arrays, 
 *          vectors, quaterions, Euler angles, strings ...).
 * 
 * @see @ref md_pages_Logging
 * @see @ref Logger-example.cpp
 */

/**
 * @brief   A class for serializing key-value/array data for logging and 
 *          sending to the GUI.
 * 
 * The Logger class provides an easy to use Logger::log method that can be used 
 * with single values/objects, strings, arrays of values/objects, and vectors of
 * values/objects.
 * 
 * ~~~cpp
 * int values[] = {1, 2, 3};
 * MainLogger.log("key", values);
 * ~~~
 * 
 * Have a look at [**Logger-example.cpp**](Logger-example_8cpp-example.html) for 
 * more examples.
 * 
 * @see @ref md_pages_Logging
 * 
 * **Data structure**
 * 
 * The log entry is just a long array of bytes. Each key-value pair in the entry
 * starts with a 4-byte header that contains a unique identifier of the type of
 * the data, the length of the key (in bytes) and the length of the data (in
 * bytes). 
 *     
 *          0        1        2        3
 *     +--------+--------+--------+--------+
 *     | id len |  type  |   data length   |
 *     +--------+--------+--------+--------+
 *     |             identifier            |
 *     +--------+--------+--------+--------+
 *     |                ...                |
 *     +--------+--------+--------+--------+
 *     |        identifier        |  NULL  |
 *     +--------+--------+--------+--------+
 *     |               data                |
 *     +--------+--------+--------+--------+
 *     |                ...                |
 *     +--------+--------+--------+--------+
 *     
 *     └─1 byte─┘ 
 *     └──────────────1 word───────────────┘
 * 
 * The header is always aligned to a word boundary.
 * The identifier starts at the second word.
 * There is at least one NULL byte after the identifier, and at most four.
 * The data always starts at a word boundary,
 * right after the null bytes after the identifier. The data can be a single 
 * object, or an array of objects, without any padding.
 * 
 * The type identifier is defined using the LoggableType structs. These structs
 * also define how objects of that type should be copied into or from the 
 * buffer, as well as the size of a single object.
 * 
 * There are two main types of loggable data: fundamental types (int, float, 
 * bool, etc.) that can be `memcpy`d into or from the buffer, and user-defined
 * types that have special functions for copying their data.  
 * The former are defined in @ref LoggerTypes.hpp, and an example of the latter 
 * can be found in @ref LoggerMathTypes.hpp.
 * 
 * Structured data cannot be `memcpy`d, because the compiler doesn't necessarily
 * pack all of the data together, it's free to allow padding bytes where 
 * necessary. ([Wikipedia: Data structure alignment](https://en.wikipedia.org/wiki/Data_structure_alignment))
 */
class Logger {
  public:
    Logger(uint8_t *buffer, size_t bufferSize)
        : buffer(buffer), bufferSize(bufferSize) {}

    /**
     * @brief   Add a log element to the log, or update the existing element
     *          with the same identifier.
     * 
     * If the identifier can't be found in the LogEntry, the new element is 
     * append%ed. If an element with the same identifier is found in the 
     * LogEntry, and if the type and size are the same, the existing element is
     * updated with the new value. If the type or size don't match, the log is
     * not altered.
     * 
     * @tparam  T 
     *          The type of the value to log.
     * @param   identifier 
     *          The identifier / key for this element.
     *          The maximum length is 255 characters.
     * @param   data
     *          A pointer to the (array of) value(s) to log.
     * @param   count
     *          The number of elements in the array.
     *          The maximum size is 65535 bytes, so the maximum count is 65535
     *          divided by the size of one element.
     * @retval  true
     *          The element was added to the log successfully, or the existing 
     *          element with the same identifier was updated with the new value. 
     * @retval  false 
     *          The buffer is full, the identifier or data length is too large, 
     *          or the type and size don't match the ones of the existing 
     *          element with the same identifier.
     */
    template <class T>
    bool log(const char *identifier, const T *data, size_t count) {
        // don't allow empty identifiers
        if (identifier == nullptr || identifier[0] == '\0')
            return false;
        // don't allow null data
        if (data == nullptr && count != 0)
            return false;
        // Check if log entry already contains a value with this identifier
        LogEntryIterator::iterator found = find(identifier);
        if (found != LogEntryIterator::end())
            // if the logentry already contains an element with the same id
            return overwrite(found, data, count);
        else
            // this is a new id
            return append(identifier, data, count);
    }

    /**
     * @brief   Add a log element to the log, or update the existing element
     *          with the same identifier. The data of the element is a single 
     *          value.
     * 
     * If the identifier can't be found in the LogEntry, the new element is 
     * append%ed. If an element with the same identifier is found in the 
     * LogEntry, and if the type and size are the same, the existing element is
     * updated with the new value. If the type or size don't match, the log is
     * not altered.
     * 
     * @tparam  T
     *          The type of the value to log.
     * @param   identifier 
     *          The identifier / key for this element.
     *          The maximum length is 255 characters.
     * @param   value
     *          The value to log.
     *          The maximum size is 65535 bytes.
     * @retval  true
     *          The element was added to the log successfully, or the existing 
     *          element with the same identifier was updated with the new value. 
     * @retval  false 
     *          The buffer is full, the identifier or data length is too large, 
     *          or the type and size don't match the ones of the existing 
     *          element with the same identifier.
     */
    template <class T>
    bool log(const char *identifier, const T &value) {
        return log(identifier, &value, 1);
    }

    /**
     * @brief   Add a log element to the log, or update the existing element
     *          with the same identifier. The data of the element is a 
     *          null-terminated string.
     * 
     * If the identifier can't be found in the LogEntry, the new element is 
     * append%ed. If an element with the same identifier is found in the 
     * LogEntry, and if the type and size are the same, the existing element is
     * updated with the new value. If the type or size don't match, the log is
     * not altered.
     * 
     * @param   identifier 
     *          The identifier / key for this element.
     *          The maximum length is 255 characters.
     * @param   data 
     *          The string to log.
     *          The maximum length is 65535 characters.
     * @retval  true
     *          The element was added to the log successfully, or the existing 
     *          element with the same identifier was updated with the new value. 
     * @retval  false 
     *          The buffer is full, the identifier or data length is too large, 
     *          or the type and size don't match the ones of the existing 
     *          element with the same identifier.
     */
    bool log(const char *identifier, const char *data) {
        return log(identifier, data, strlen(data));
    }

    /**
     * @brief   Add a log element to the log, or update the existing element
     *          with the same identifier. The data of the element is an array 
     *          of values.
     * 
     * If the identifier can't be found in the LogEntry, the new element is 
     * append%ed. If an element with the same identifier is found in the 
     * LogEntry, and if the type and size are the same, the existing element is
     * updated with the new value. If the type or size don't match, the log is
     * not altered.
     * 
     * @tparam  T
     *          The type of the values to log.
     * @tparam  N
     *          The number of values to log.
     * @param   identifier 
     *          The identifier / key for this element.
     *          The maximum length is 255 characters.
     * @param   array 
     *          The array of values to log.
     *          The maximum size is 65535 bytes.
     * @retval  true
     *          The element was added to the log successfully, or the existing 
     *          element with the same identifier was updated with the new value. 
     * @retval  false 
     *          The buffer is full, the identifier or data length is too large, 
     *          or the type and size don't match the ones of the existing 
     *          element with the same identifier.
     */
    template <class T, size_t N>
    bool log(const char *identifier, const T (&array)[N]) {
        return log(identifier, array, N);
    }

    /**
     * @brief   Add a log element to the log, or update the existing element
     *          with the same identifier. The data of the element is a list 
     *          of values.
     * 
     * If the identifier can't be found in the LogEntry, the new element is 
     * append%ed. If an element with the same identifier is found in the 
     * LogEntry, and if the type and size are the same, the existing element is
     * updated with the new value. If the type or size don't match, the log is
     * not altered.
     * 
     * @tparam  T
     *          The type of the values to log.
     * @param   identifier 
     *          The identifier / key for this element.
     *          The maximum length is 255 characters.
     * @param   list 
     *          The list of values to log.
     *          The maximum size is 65535 bytes.
     * @retval  true
     *          The element was added to the log successfully, or the existing 
     *          element with the same identifier was updated with the new value. 
     * @retval  false 
     *          The buffer is full, the identifier or data length is too large, 
     *          or the type and size don't match the ones of the existing 
     *          element with the same identifier.
     */
    template <class T>
    bool log(const char *identifier, std::initializer_list<T> list) {
        return log(identifier, list.begin(), list.size());
    }

    /**
     * @brief   Add a log element to the log, or update the existing element
     *          with the same identifier. The data of the element is an 
     *          std::array of values.
     * 
     * If the identifier can't be found in the LogEntry, the new element is 
     * append%ed. If an element with the same identifier is found in the 
     * LogEntry, and if the type and size are the same, the existing element is
     * updated with the new value. If the type or size don't match, the log is
     * not altered.
     * 
     * @tparam  T
     *          The type of the values to log.
     * @tparam  N
     *          The number of values to log.
     * @param   identifier 
     *          The identifier / key for this element.
     *          The maximum length is 255 characters.
     * @param   array 
     *          The array of values to log.
     *          The maximum size is 65535 bytes.
     * @retval  true
     *          The element was added to the log successfully, or the existing 
     *          element with the same identifier was updated with the new value. 
     * @retval  false 
     *          The buffer is full, the identifier or data length is too large, 
     *          or the type and size don't match the ones of the existing 
     *          element with the same identifier.
     */
    template <class T, size_t N>
    bool log(const char *identifier, const std::array<T, N> &array) {
        return log(identifier, array.data(), N);
    }

    /**
     * @brief   Add a log element to the log, or update the existing element
     *          with the same identifier. The data of the element is an 
     *          std::vector of values.
     * 
     * If the identifier can't be found in the LogEntry, the new element is 
     * append%ed. If an element with the same identifier is found in the 
     * LogEntry, and if the type and size are the same, the existing element is
     * updated with the new value. If the type or size don't match, the log is
     * not altered.
     * 
     * @tparam  T
     *          The type of the values to log.
     * @param   identifier 
     *          The identifier / key for this element.
     *          The maximum length is 255 characters.
     * @param   vector 
     *          The vector of values to log.
     *          The maximum size is 65535 bytes.
     * @retval  true
     *          The element was added to the log successfully, or the existing 
     *          element with the same identifier was updated with the new value. 
     * @retval  false 
     *          The buffer is full, the identifier or data length is too large, 
     *          or the type and size don't match the ones of the existing 
     *          element with the same identifier.
     */
    template <class T>
    bool log(const char *identifier, const std::vector<T> &vector) {
        return log(identifier, vector.data(), vector.size());
    }

    /**
     * @brief   Clear all elements of the log.
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
     * @brief   Dump the log buffer to the given output stream in a 
     *          human-readable format (offset + hexadecimal + ASCII).
     * 
     * @param   os
     *          The stream to print to.
     */
    void print(Print &os) const;

    /**
     * @brief   Dump the log buffer to the given output stream as a Python bytes
     *          object.
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
    /// in that case, the write pointer is unchanged.
    uint8_t *writeHeader(const char *identifier, uint8_t typeID, size_t length);

    /// Append the new element to the buffer.
    /// Returns false if the element is too large for the buffer.
    template <class T>
    bool append(const char *identifier, const T *data, size_t count);

    /// Overwrite the existing log element referenced by @p existing with the
    /// new data, if the type and size match.
    /// Returns false if the type or size doesn't match, and in that case,
    /// nothing is overwritten.
    template <class T>
    bool overwrite(LogEntryIterator::iterator existing, const T *data,
                   size_t count);

    /// (Over)write the data of a log element to the buffer.
    template <class T>
    void overwrite(uint8_t *buffer, const T *data, size_t count);

  public:
    /// Get the buffer containing the logging data.
    const uint8_t *getBuffer() const { return buffer; }
    /// Get the total size of the buffer.
    size_t getBufferSize() const { return bufferSize; }
    /// Get the length of the used part of the buffer.
    size_t getLength() const { return getBufferSize() - maxLen; }

    /** 
     * @brief   Get the element with the given key.
     * 
     * @param   key
     *          The unique identifier / key of the element to retreive.
     * @return  An iterator to the element with the given key, or a 
     *          default-constructed LogEntryIterator::iterator if the key was 
     *          not found in the log.
     */
    LogEntryIterator::iterator find(const char *key) const;
};

/// Macro for easily logging variables with the variable name as the identifier.
#define LOG_VAR(logger, var) logger.log(#var, var)

/// Logger with a static buffer
template <size_t N>
class StaticLogger : public Logger {
  public:
    StaticLogger() : Logger(buffer.data(), N) {}

  private:
    std::array<uint8_t, N> buffer = {{}};
};

/// @}

#include <KVComm/src/Logger.ipp>