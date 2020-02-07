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

    /**
     * @brief   Receives a packet into the read buffer.
     * 
     * If more than len bytes are received, the packet will be truncated.
     * 
     * @return  The number of bytes stored in the buffer.
     * 
     * @see     setReadBuffer
     */
    size_t readPacket();

    /**
     * @brief   Check if the received packet was truncated
     */
    bool wasTruncated() const { return parser.wasTruncated(); }

  private:
    Stream *stream;
    SLIPParser parser;
};

template <class CRC>
class SLIPStreamCRC {
  public:
    struct StreamSender {
        StreamSender(Stream *stream) : stream(stream) {}
        StreamSender(Stream &stream) : stream(&stream) {}
        size_t operator()(uint8_t c) const { return stream->write(c); }

        Stream *stream;
    };

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
     *          of the delimiters and stuffing bytes.
     */
    size_t writePacket(const uint8_t *data, size_t len);

    /**
     * @brief   Receives a packet into the read buffer.
     * 
     * If more than len bytes are received, the packet will be truncated.
     * 
     * @return  The number of bytes stored in the buffer.
     * 
     * @see     setReadBuffer
     */
    size_t readPacket();

    /**
     * @brief   Check if the received packet was truncated
     */
    bool wasTruncated() const { return parser.wasTruncated(); }

    /**
     * @brief   The type of the CRC checksum.
     */
    using checksum_t = typename SLIPParserCRC<CRC>::checksum_t;

    /**
     * @brief   Get the CRC checksum of the parser. Should be zero if the
     *          packet was received correctly.
     */
    checksum_t checksum() const { return parser.checksum(); }

  private:
    Stream *stream;
    SLIPSenderCRC<StreamSender, CRC> sender;
    SLIPParserCRC<CRC> parser;
};

/// @}

#include "SLIPStream.ipp"