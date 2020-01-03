#pragma once

#ifndef ARDUINO

#include <KVComm/public/LoggerTypes.hpp>  // LoggableType

#include <cstdint>    // uint8_t, uint32_t
#include <map>        // std::map
#include <stdexcept>  // std::logic_error, std::out_of_range, std::length_error
#include <string>     // std::string
#include <vector>     // std::vector

/// @addtogroup logger
/// @{

/**
 * @file  
 * @brief   A parser for log entries generated by the Logger (intended to run
 *          on your computer or on the Zybo Linux core, not on the Baremetal 
 *          core).
 */

/// Class for parsing a LogEntry from a buffer of bytes
class ParsedLogEntry {
  public:
    /// Struct for accessing the information of a single element of the LogEntry
    struct LogElement {
        /// A pointer to the raw data
        const uint8_t *data;
        /// The type identifier (tells you what the type of the data is)
        uint8_t type : 8;
        /// The length of the data (in bytes)
        uint32_t length : 24;

        /**
         * @brief   Get the data of the element as the given type.
         * 
         * @tparam  T 
         *          The type of the data. This must be the same as the dynamic
         *          type of the data.
         * @param   index 
         *          The array index of the value to extract.
         * @return  The value that was saved at the given index of the element,
         *          converted to the correct type.
         * @throw   std::logic_error
         *          The dynamic type ID doesn't match the type @p T.
         * @throw   std::out_of_range
         *          The array index is out of bounds.
         */
        template <class T>
        T get(size_t index = 0) const {
            if (!this->hasType<T>())
                throw std::logic_error("Invalid type");
            if (index * LoggableType<T>::getLength() >= this->length)
                throw std::out_of_range("Index out of range");
            auto readlocation = data + LoggableType<T>::getLength() * index;
            return LoggableType<T>::readFromBuffer(readlocation);
        }

        /**
         * @brief   Get the data of the element as a vector of the given type.
         * 
         * @tparam  T 
         *          The type of the data. This must be the same as the dynamic
         *          type of the data.
         * @return  A vector containing the data of the element, converted to 
         *          the correct type.
         * @throw   std::logic_error
         *          The dynamic type ID doesn't match the type @p T.
         */
        template <class T>
        std::vector<T> getVector() const {
            if (!this->hasType<T>())
                throw std::logic_error("Invalid type");
            size_t size = this->length / LoggableType<T>::getLength();
            std::vector<T> result(size);
            auto readlocation = data;
            for (T &t : result) {
                t = LoggableType<T>::readFromBuffer(readlocation);
                readlocation += LoggableType<T>::getLength();
            }
            return result;
        }

        /**
         * @brief   Get the data of the element as an array of the given type.
         * 
         * @tparam  T 
         *          The type of the data. This must be the same as the dynamic
         *          type of the data.
         * @tparam  N
         *          The number of elements in the array. This must be the same
         *          as the length of the data.
         * @return  An array containing the data of the element, converted to 
         *          the correct type.
         * @throw   std::logic_error
         *          The dynamic type ID doesn't match the type @p T.
         * @throw   std::length_error
         *          The array size doesn't match the actual size of the element.
         */
        template <class T, size_t N>
        std::array<T, N> getArray() const {
            if (!this->hasType<T>())
                throw std::logic_error("Invalid type");
            if (N * LoggableType<T>::getLength() != this->length)
                throw std::length_error("Incorrect length");
            std::array<T, N> result;
            auto readlocation = data;
            for (T &t : result) {
                t = LoggableType<T>::readFromBuffer(readlocation);
                readlocation += LoggableType<T>::getLength();
            }
            return result;
        }

        /**
         * @brief   Get the character array as an std::string.
         * 
         * @return  The character array as a string.
         * @throw   std::logic_error
         *          The dynamic type of the element is not @c char.
         */
        std::string getString() const;

        /**
         * @brief   Check if the type of this element is the same as the given 
         *          type.
         * 
         * @tparam  T
         *          The type to compare to the type of this element.
         * @return  True if the types are the same, false otherwise.
         */
        template <class T>
        bool hasType() const {
            return this->type == LoggableType<T>::getTypeID();
        }
    };

  private:
    /// Functor to compare the map keys
    struct strcmp {
        bool operator()(const char *a, const char *b) const;
    };
    using map_t = std::map<const char *, LogElement, strcmp>;
    map_t parseResult{};

  public:
    /**
     * @brief   Parse a raw buffer into a new ParsedLogEntry.
     * 
     * @param   buffer
     *          A pointer to the raw buffer containing the LogEntry.
     *          Only the pointer is saved, no copy is made of the actual buffer,
     *          so the lifetime of the buffer must be longer than the parser and
     *          the LogElement accessors.
     * @param   length
     *          The length of the LogEntry in the buffer (in bytes).
     */
    ParsedLogEntry(const uint8_t *buffer, size_t length)
        : parseResult{parse(buffer, length)} {}

    /**
     * @brief   Check if the LogEntry contains an element with the given 
     *          identifier/key.
     * 
     * @param   key
     *          The identifier of the element to check. 
     * @return  true if the LogEntry contains an element with the given 
     *          identifier, false if it doesn't.
     */
    bool contains(const char *key) const {
        return parseResult.find(key) != parseResult.end();
    }

    /**
     * @brief   Get the element with the given identifier/key.
     * 
     * @param   key 
     *          The identifier of the element to retreive.
     * @return  The element with the given identifier.
     * @throw   std::out_of_range
     *          There is no element with the given identifier.
     */
    LogElement getElement(const char *key) const { return parseResult.at(key); }
    /// @copydoc getElement
    LogElement operator[](const char *key) const { return getElement(key); }
    /// Begin iterator over all LogElements in the LogEntry
    map_t::iterator begin() { return parseResult.begin(); }
    /// Begin iterator over all LogElements in the LogEntry
    map_t::const_iterator begin() const { return parseResult.begin(); }
    /// End iterator over all LogElements in the LogEntry
    map_t::iterator end() { return parseResult.end(); }
    /// End iterator over all LogElements in the LogEntry
    map_t::const_iterator end() const { return parseResult.end(); }

  private:
    /**
     * @brief   Parse the buffer for LogElements, save their identifiers, type,
     *          and size in an std::map, together with a pointer to the actual
     *          data. 
     * 
     * @param   buffer 
     *          A pointer to the raw buffer containing the LogEntry.
     * @param   length 
     *          The length of the LogEntry in the buffer (in bytes).
     * @return  A map mapping the identifiers to the LogElements.
     */
    map_t parse(const uint8_t *buffer, size_t length);
};

/// @}

#endif // ARDUINO