#include "SLIPSender.hpp"

template <class Sender>
size_t SLIPSender<Sender>::write(const uint8_t *data, size_t len) {
    using namespace SLIP_Constants;

    size_t sent = 0;

    /* 
     * for each byte in the packet, send the appropriate character
     * sequence
     */
    while (len--) {
        switch (*data) {
            /*
             * if it's the same code as an END character, we send a
             * special two character code so as not to make the
             * receiver think we sent an END
             */
            case END:
                sent += this->sender(ESC);
                sent += this->sender(ESC_END);
                break;

            /*
             * if it's the same code as an ESC character,
             * we send a special two character code so as not
             * to make the receiver think we sent an ESC
             */
            case ESC:
                sent += this->sender(ESC);
                sent += this->sender(ESC_ESC);
                break;
            /*
             * otherwise, we just send the character
             */
            default: sent += this->sender(*data);
        }

        data++;
    }

    return sent;
}

template <class Sender, class CRC>
size_t SLIPSenderCRC<Sender, CRC>::write(const uint8_t *data, size_t len) {
    this->crc.process_bytes(data, len);
    return sender.write(data, len);
}