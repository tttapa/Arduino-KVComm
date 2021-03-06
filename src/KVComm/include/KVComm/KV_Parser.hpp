#pragma once

#if !defined(ARDUINO) || defined(DOXYGEN)

#include <KVComm/KV_Types.hpp>  // KV_Type

#include <cstdint>    // uint8_t, uint32_t
#include <map>        // std::map
#include <stdexcept>  // std::logic_error, std::out_of_range, std::length_error
#include <string>     // std::string
#include <vector>     // std::vector

/// @addtogroup KVComm
/// @{

/**
 * @file  
 * @brief   A parser for dictionaries generated by the @ref KV_Builder.
 * 
 * Intended to run on your computer, not on the Arduino, because it uses 
 * `std::map`, dynamic memory allocation and exceptions.
 */

/**
 * @brief   A parser for dictionaries generated by the @ref KV_Builder.
 * 
 * Intended to run on your computer, not on the Arduino, because it uses 
 * `std::map`, dynamic memory allocation and exceptions.
 */
class KV_Parser {
  private:
    /// Functor to compare the map keys
    struct strcmp {
        bool operator()(const char *a, const char *b) const;
    };
    using KV    = KV_Iterator::KV;
    using map_t = std::map<const char *, KV, strcmp>;

    map_t parseResult{};

  public:
    /**
     * @brief   Parse a raw buffer into a new KV_Parser.
     * 
     * @param   buffer
     *          A pointer to the raw buffer containing the dictionary, as 
     *          generated by a @ref KV_Builder.  
     *          Only the pointer is saved, no copy is made of the actual buffer,
     *          so the lifetime of the buffer must be longer than the parser and
     *          the accessors.
     * @param   length
     *          The length of the dictionary in the buffer (in bytes).
     */
    KV_Parser(const uint8_t *buffer, size_t length)
        : parseResult{parse(buffer, length)} {}

    /**
     * @brief   Check if the dictionary contains an element with the given key.
     * 
     * @param   key
     *          The key of the element to check. 
     * @retval  true
     *          The dictionary contains an element with the given key.
     * @retval  false
     *          Otherwise.
     */
    bool contains(const char *key) const {
        return parseResult.find(key) != parseResult.end();
    }

    /**
     * @brief   Get the element with the given key.
     * 
     * @param   key 
     *          The key of the element to retreive.
     * @return  The element with the given key.
     * @throw   std::out_of_range
     *          There is no element with the given key.
     */
    KV getElement(const char *key) const { return parseResult.at(key); }
    /// @copydoc getElement
    KV operator[](const char *key) const { return getElement(key); }
    /// Begin iterator over all entries in the dictionary.
    map_t::iterator begin() { return parseResult.begin(); }
    /// Begin iterator over all entries in the dictionary.
    map_t::const_iterator begin() const { return parseResult.begin(); }
    /// End iterator over all entries in the dictionary.
    map_t::iterator end() { return parseResult.end(); }
    /// End iterator over all entries in the dictionary.
    map_t::const_iterator end() const { return parseResult.end(); }

  private:
    /**
     * @brief   Parse the buffer for key-value pairs, save their keys, type,
     *          and size in an std::map, together with a pointer to the actual
     *          value.
     * 
     * @param   buffer 
     *          A pointer to the raw buffer containing the dictionary.
     * @param   length 
     *          The length of the dictionary in the buffer (in bytes).
     * @return  A map mapping the keys to the key-value dictionary entries.
     */
    map_t parse(const uint8_t *buffer, size_t length);
};

/// @}

#endif  // ARDUINO