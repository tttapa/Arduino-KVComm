#pragma once

#include <AH/STL/algorithm> // std::reverse
#include <AH/STL/utility>   // std::forward
#include <limits.h>         // CHAR_BIT
#include <stddef.h>         // size_t
#include <string.h>         // memcpy

#include <boost/integer.hpp> // boost::uint_t

#include <SLIPStream/SLIP.hpp>

template <class Sender>
class SLIPSender {
  public:
    SLIPSender() = default;
    SLIPSender(Sender &&sender) : sender(std::forward<Sender>(sender)) {}

    size_t beginPacket() { return sender(SLIP_Constants::END); }
    size_t endPacket() { return sender(SLIP_Constants::END); }

    size_t write(const uint8_t *data, size_t len);

  private:
    Sender sender;
};

template <class Sender, class CRC>
class SLIPSenderCRC {
  public:
    SLIPSenderCRC() = default;
    SLIPSenderCRC(Sender &&sender) : sender(std::forward<Sender>(sender)) {}
    SLIPSenderCRC(CRC &&crc) : crc(std::forward<CRC>(crc)) {}
    SLIPSenderCRC(Sender &&sender, CRC &&crc)
        : sender(std::forward<Sender>(sender)), crc(std::forward<CRC>(crc)) {}

    using checksum_t = typename boost::uint_t<CRC::bit_count>::least;

    size_t beginPacket() {
        this->crc.reset();
        return sender.beginPacket();
    }

    size_t endPacket() {
        constexpr size_t numChars = sizeof(checksum_t);
        uint8_t buffer[numChars];
        const checksum_t checksum = this->crc.checksum();
        memcpy(buffer, &checksum, numChars);
        std::reverse(std::begin(buffer), std::end(buffer));
        return sender.write(buffer, numChars) + sender.endPacket();
    }

    size_t write(const uint8_t *data, size_t len);

  private:
    SLIPSender<Sender> sender;
    CRC crc;
};

#include "SLIPSender.ipp"
