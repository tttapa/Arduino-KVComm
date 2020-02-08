#pragma once

#include <SLIPStream/SLIPParser.hpp>
#include <SLIPStream/SLIPSender.hpp>
#include <Stream.h>

/// @addtogroup SLIP
/// @{

/**
 * @brief   Class that implements SLIP, a simple packet framing protocol.
 * 
 * [RFC 1055](https://tools.ietf.org/html/rfc1055)
 */
class SLIPStream {
  public:
    /// Functor that sends bytes over an Arduino Stream.
    struct StreamSender {
        StreamSender(Stream &stream) : stream(&stream) {}
        StreamSender(Stream *stream) : stream(stream) {}
        size_t operator()(uint8_t c) const { return stream->write(c); }
        Stream *stream;
    };

  public:
    SLIPStream(Stream &stream, const SLIPParser &parser)
        : stream(&stream), parser(parser) {}
    SLIPStream(Stream &stream) : stream(&stream), parser(nullptr, 0) {}

    /**
     * @brief   Sends a packet.
     * 
     * @param   data
     *          A pointer to the start of the data.
     * @param   len
     *          The length of the data.
     * @return  The number of bytes transmitted over the Stream. If no write 
     *          errors occur, this number will be larger than @p len, because
     *          of the delimiters and stuffing bytes.
     */
    size_t writePacket(const uint8_t *data, size_t len);

    /// @copydoc    SLIPSender::beginPacket
    size_t beginPacket();
    /// @copydoc    SLIPSender::write
    size_t write(const uint8_t *data, size_t len);
    /// @copydoc    SLIPSender::endPacket
    size_t endPacket();

    /**
     * @brief   Receives a packet into the read buffer.
     * 
     * If more than len bytes are received, the packet will be truncated.
     * 
     * @return  The number of bytes stored in the buffer.
     */
    size_t readPacket();

    /// @copydoc    SLIPParser::wasTruncated
    bool wasTruncated() const { return parser.wasTruncated(); }

    /// @copydoc    SLIPParser::numTruncated
    size_t numTruncated() const { return parser.numTruncated(); }

  private:
    Stream *stream;
    SLIPParser parser;
};

/// @}

/// @addtogroup CRC
/// @{

/**
 * @brief   Class that implements SLIP, a simple packet framing protocol, and 
 *          that uses cyclic redundancy checks (CRCs) on transmitted and 
 *          received packets.
 * 
 * @see [**RFC 1055**](https://tools.ietf.org/html/rfc1055)
 * @see [**Boost::CRC**](https://www.boost.org/doc/libs/1_72_0/doc/html/crc.html)
 */
template <class CRC>
class SLIPStreamCRC {
  public:
    using StreamSender = SLIPStream::StreamSender;

    SLIPStreamCRC(Stream &stream, CRC &&senderCRC, const SLIPParser &parser,
                  CRC &&parserCRC)
        : stream(&stream), sender(stream, std::forward<CRC>(senderCRC)),
          parser(parser, std::forward<CRC>(parserCRC)) {}

    /**
     * @brief   Sends a packet.
     * 
     * @param   data
     *          A pointer to the start of the data.
     * @param   len
     *          The length of the data.
     * @return  The number of bytes transmitted over the Stream. If no write 
     *          errors occur, this number will be larger than @p len, because
     *          of the delimiters, checksums and stuffing bytes.
     */
    size_t writePacket(const uint8_t *data, size_t len);

    /// @copydoc    SLIPSenderCRC::beginPacket
    size_t beginPacket();
    /// @copydoc    SLIPSenderCRC::write
    size_t write(const uint8_t *data, size_t len);
    /// @copydoc    SLIPSenderCRC::endPacket
    size_t endPacket();

    /// @copydoc    SLIPStream::readPacket
    size_t readPacket();

    /// @copydoc    SLIPParserCRC::wasTruncated
    bool wasTruncated() const { return parser.wasTruncated(); }
    /// @copydoc    SLIPParserCRC::numTruncated
    size_t numTruncated() const { return parser.numTruncated(); }

    /// @copydoc    SLIPParserCRC::checksum_t
    using checksum_t = typename SLIPParserCRC<CRC>::checksum_t;

    /// @copydoc    SLIPParserCRC::checksum
    checksum_t checksum() const { return parser.checksum(); }

  private:
    Stream *stream;
    SLIPSenderCRC<StreamSender, CRC> sender;
    SLIPParserCRC<CRC> parser;
};

/// @}

#include "SLIPStream.ipp"