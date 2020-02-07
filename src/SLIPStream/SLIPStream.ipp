#include "SLIPStream.hpp"

#include <SLIPStream/SLIPSender.hpp>

template <class CRC>
size_t SLIPStreamCRC<CRC>::writePacket(const uint8_t *data, size_t len) {
    size_t sent = 0;

    sent += sender.beginPacket();
    sent += sender.write(data, len);
    sent += sender.endPacket();

    return sent;
}

template <class CRC>
size_t SLIPStreamCRC<CRC>::readPacket() {
    while (stream->available()) {
        size_t packetSize = parser.parse(stream->read());
        if (packetSize > 0)
            return packetSize;
    }
    return 0;
}
