#include "SLIPStream.hpp"

#include <SLIPStream/SLIPSender.hpp>

template <class CRC>
size_t SLIPStreamCRC<CRC>::writePacket(const uint8_t *data, size_t len) {
    size_t sent = 0;
    sent += beginPacket();
    sent += write(data, len);
    sent += endPacket();
    return sent;
}

template <class CRC>
size_t SLIPStreamCRC<CRC>::write(const uint8_t *data, size_t len) {
    return sender.write(data, len);
}

template <class CRC>
size_t SLIPStreamCRC<CRC>::beginPacket() {
    return sender.beginPacket();
}

template <class CRC>
size_t SLIPStreamCRC<CRC>::endPacket() {
    return sender.endPacket();
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
