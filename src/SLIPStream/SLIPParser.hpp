#pragma once

#include <AH/STL/utility> // std::forward
#include <stddef.h>       // size_t

#include <SLIPStream/SLIP.hpp>

#include <boost/integer.hpp>

class SLIPParser {
  public:
    SLIPParser(uint8_t *buffer, size_t bufferSize)
        : buffer(buffer), bufferSize(bufferSize) {
        reset();
    }

    template <class Callback>
    size_t parse(uint8_t c, Callback callback);

    size_t parse(uint8_t c) {
        auto cb = [](uint8_t, size_t) {};
        return parse(c, cb);
    }

    bool wasTruncated() const { return truncated; }

  private:
    void reset() { write = buffer; }

  private:
    uint8_t *buffer   = nullptr;
    uint8_t *write    = nullptr;
    size_t bufferSize = 0;
    bool truncated    = false;
    bool escape       = false;
};

template <class CRC>
class SLIPParserCRC {
  public:
    SLIPParserCRC(const SLIPParser &parser) : parser(parser) {}
    SLIPParserCRC(const SLIPParser &parser, CRC &&crc)
        : parser(parser), crc(std::forward<CRC>(crc)) {}

    using checksum_t = typename boost::uint_t<CRC::bit_count>::least;

    size_t parse(uint8_t c) {
        auto cb = [this](uint8_t c, size_t index) {
            if (index == 0)
                crc.reset();
            crc(c);
        };
        size_t size = parser.parse(c, cb);
        return size < sizeof(checksum_t) ? 0 : size - sizeof(checksum_t);
    }

    bool wasTruncated() const { return parser.wasTruncated(); }
    checksum_t checksum() const { return crc.checksum(); }

  private:
    SLIPParser parser;
    CRC crc;
};

#include "SLIPParser.ipp"
