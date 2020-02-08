#pragma once

#include <AH/STL/cstddef> // std::nullptr, size_t
#include <AH/STL/utility> // std::forward
#include <stddef.h>       // size_t

#include <SLIPStream/SLIP.hpp>

#include <boost/integer.hpp>

/// @addtogroup SLIP
/// @{

/**
 * @brief   Class for parsing SLIP packets.
 */
class SLIPParser {
  public:
    /** 
     * @brief   Constructor.
     * 
     * @param   buffer
     *          The byte buffer to store the parsed packets.
     */
    template <size_t N>
    SLIPParser(uint8_t (&buffer)[N]) : SLIPParser(buffer, N) {}

    /**
     * @brief   Default constructor for a parser without a buffer.
     */
    SLIPParser() : SLIPParser(nullptr) {}

    /**
     * @brief   Constructor for a parser without a buffer.
     */
    SLIPParser(std::nullptr_t) : SLIPParser(nullptr, 0) {}

    /** 
     * @brief   Constructor.
     * 
     * @param   buffer
     *          The byte buffer to store the parsed packets.
     * @param   bufferSize
     *          The size of the buffer.
     */
    SLIPParser(uint8_t *buffer, size_t bufferSize)
        : buffer(buffer), bufferSize(bufferSize) {
        reset();
    }

    /**
     * @brief   Parse the given byte, and call the callback for each data byte.
     * 
     * @tparam  Callback 
     *          The type of callback function. Should be a callable object that 
     *          takes a data byte and the index of that byte in the packet:  
     *          `void callback(uint8_t databyte, size_t index)`
     *          
     * @param   c
     *          The byte to parse.
     * @param   callback 
     *          The callback function to call for each data byte.
     * @retval  0
     *          The packet is not finished yet.
     * @retval >0
     *          The packet was received in its entirety, and the return value is
     *          the size of the packet in the buffer.  
     *          If the packet is not larger than the buffer, this will be the 
     *          same as the size of the packet. If the packet was larger than 
     *          the buffer, the return value will be the size of the buffer, and
     *          @ref wasTruncated will return true.
     */
    template <class Callback>
    size_t parse(uint8_t c, Callback callback);

    /**
     * @brief   Parse the given byte without using a callback function.
     * 
     * @param   c
     *          The byte to parse.
     * @retval  0
     *          The packet is not finished yet.
     * @retval >0
     *          The packet was received in its entirety, and the return value is
     *          the size of the packet in the buffer.  
     *          If the packet is not larger than the buffer, this will be the 
     *          same as the size of the packet. If the packet was larger than 
     *          the buffer, the return value will be the size of the buffer, and
     *          @ref wasTruncated will return true.
     */
    size_t parse(uint8_t c) {
        auto cb = [](uint8_t, size_t) {};
        return parse(c, cb);
    }

    /**
     * @brief   Check if the previous packet was truncated.
     * 
     * @retval  true
     *          The previous packet didn't fit the buffer. The size returned by
     *          @ref SLIPParser::parse was smaller than the actual size of the 
     *          packet.
     * @retval  false 
     *          The buffer was large enough to store the previous packet.
     */
    bool wasTruncated() const { return truncated > 0; }

    /**
     * @brief   Get the number of bytes that were truncated due to the previous
     *          packet being too large for the buffer.
     */
    size_t numTruncated() const { return truncated; }

  private:
    void reset() { write = buffer; }

  private:
    uint8_t *buffer   = nullptr;
    uint8_t *write    = nullptr;
    size_t bufferSize = 0;
    size_t truncated  = 0;
    bool escape       = false;
};

/// @}

/// @addtogroup CRC
/// @{

/**
 * @brief   Class for parsing SLIP packets with a CRC checksum to check the 
 *          integrity of the packets.
 * 
 * @tparam  CRC 
 *          The CRC type to use.
 */
template <class CRC>
class SLIPParserCRC {
  public:
    SLIPParserCRC(const SLIPParser &parser) : parser(parser) {}
    SLIPParserCRC(const SLIPParser &parser, CRC &&crc)
        : parser(parser), crc(std::forward<CRC>(crc)) {}

    /// The integer type of the checksum.
    using checksum_t = typename boost::uint_t<CRC::bit_count>::least;

    /**
     * @brief   Parse the given byte.
     * 
     * @param   c
     *          The byte to parse.
     * 
     * @retval  0
     *          The packet is not finished yet.
     * @retval >0
     *          The packet was received in its entirety, and the return value is
     *          the size of the packet in the buffer.  
     *          If the packet is not larger than the buffer, this will be the 
     *          same as the size of the packet. If the packet was larger than 
     *          the buffer, the return value will be the size of the buffer, and
     *          @ref wasTruncated will return true.
     */
    size_t parse(uint8_t c) {
        // callback that resets the CRC when necessary, and that feeds the 
        // parsed byte to the CRC
        auto cb = [this](uint8_t c, size_t index) {
            if (index == 0)
                crc.reset();
            crc(c);
        };
        // use the standard SLIPParser
        size_t size = parser.parse(c, cb);

        // Correct the size of the packet for the size of the checksum
        if (size <= sizeof(checksum_t))
            return 0;
        if (parser.numTruncated() < sizeof(checksum_t))
            return size + parser.numTruncated() - sizeof(checksum_t);
        return size;
    }

    /**
     * @brief   Check if the previous packet was truncated.
     * 
     * @retval  true
     *          The previous packet didn't fit the buffer. The size returned by
     *          @ref SLIPParser::parse was smaller than the actual size of the 
     *          packet.
     * @retval  false 
     *          The buffer was large enough to store the previous packet.
     */
    bool wasTruncated() const { return numTruncated() > 0; }

    /**
     * @brief   Get the number of bytes that were truncated due to the previous
     *          packet being too large for the buffer.
     */
    size_t numTruncated() const {
        return parser.numTruncated() < sizeof(checksum_t)
                   ? 0
                   : parser.numTruncated() - sizeof(checksum_t);
    }
    /**
     * @brief   Get the checksum of the previous packet. A checksum of zero 
     *          indicates that the packet was received correctly.
     */
    checksum_t checksum() const { return crc.checksum(); }

  private:
    SLIPParser parser;
    CRC crc;
};

/// @}

#include "SLIPParser.ipp"
