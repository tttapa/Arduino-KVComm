#pragma once

#include <AH/STL/algorithm> // std::reverse
#include <AH/STL/utility>   // std::forward
#include <limits.h>         // CHAR_BIT
#include <stddef.h>         // size_t
#include <string.h>         // memcpy

#include <boost/integer.hpp> // boost::uint_t

#include <SLIPStream/SLIP.hpp>

/// @addtogroup SLIP
/// @{

/**
 * @brief   Class for sending SLIP packets.
 * 
 * @tparam  Sender 
 *          The functor that actually sends a byte over the transmission 
 *          channel. Takes a single byte as an argument and returns the number
 *          of bytes written (similar to `Serial.write(uint8_t)`):  
 *          `size_t sender(uint8_t byte)`
 */
template <class Sender>
class SLIPSender {
  public:
    /**
     * @brief   Default constructor.
     */
    SLIPSender() = default;
    /**
     * @brief   Constructor with sender initialization.
     * 
     * @param   sender 
     *          Initialization for the sender. Perfect forwarding is used.
     */
    SLIPSender(Sender &&sender) : sender(std::forward<Sender>(sender)) {}

    /**
     * @brief   Start a packet.
     * 
     * Sends a SLIP @ref SLIP_Constants::END "END" character to flush the buffer
     * of the receiver.
     * 
     * @return  The number of bytes sent by the Sender.
     */
    size_t beginPacket() { return sender(SLIP_Constants::END); }
    /**
     * @brief   Finish the packet.
     * 
     * Sends a SLIP @ref SLIP_Constants::END "END" character.
     * 
     * @return  The number of bytes sent by the Sender.
     */
    size_t endPacket() { return sender(SLIP_Constants::END); }

    /**
     * @brief   Write some data as the body of a packet.
     * 
     * The data is encoded by SLIP before sending, so arbitrary binary data can
     * be sent.
     * 
     * @param   data 
     *          A pointer to the data to send.
     * @param   len 
     *          The number of bytes to send.
     * @return  The number of bytes sent by the Sender.
     */
    size_t write(const uint8_t *data, size_t len);

  private:
    Sender sender;
};

/// @}

/// @addtogroup CRC
/// @{

/**
 * @brief   Class for sending SLIP packets with a CRC checksum to check the 
 *          integrity of the packets.
 * 
 * @tparam  Sender 
 *          The functor that actually sends a byte over the transmission 
 *          channel. Takes a single byte as an argument and returns the number
 *          of bytes written (similar to `Serial.write(uint8_t)`):  
 *          `size_t sender(uint8_t byte)`
 * @tparam  CRC 
 *          The CRC type to use.
 */
template <class Sender, class CRC>
class SLIPSenderCRC {
  public:
    /**
     * @brief   Default constructor.
     */
    SLIPSenderCRC() = default;
    /**
     * @brief   Constructor with sender initialization.
     * 
     * @param   sender 
     *          Initialization for the sender. Perfect forwarding is used.  
     *          The CRC is default-initialized.
     */
    SLIPSenderCRC(Sender &&sender) : sender(std::forward<Sender>(sender)) {}
    /**
     * @brief   Constructor with CRC initialization.
     * 
     * @param   crc 
     *          Initialization for the CRC. Perfect forwarding is used.  
     *          The sender is default-initialized.
     */
    SLIPSenderCRC(CRC &&crc) : crc(std::forward<CRC>(crc)) {}
    /**
     * @brief   Constructor with sender and CRC initialization.
     * 
     * @param   sender 
     *          Initialization for the sender. Perfect forwarding is used.  
     * @param   crc 
     *          Initialization for the CRC. Perfect forwarding is used.  
     */
    SLIPSenderCRC(Sender &&sender, CRC &&crc)
        : sender(std::forward<Sender>(sender)), crc(std::forward<CRC>(crc)) {}

    /// The integer type of the checksum.
    using checksum_t = typename boost::uint_t<CRC::bit_count>::least;

    /// @copydoc    SLIPSender::beginPacket()
    size_t beginPacket() {
        this->crc.reset();
        return sender.beginPacket();
    }

    /**
     * @brief   Finish the packet.
     * 
     * Encodes and sends the checksum of all data sent using the @ref write
     * function, followed by a SLIP @ref SLIP_Constants::END "END" character.
     * 
     * @return  The number of bytes sent by the Sender.
     */
    size_t endPacket() {
        constexpr size_t numChars = sizeof(checksum_t);
        uint8_t buffer[numChars];
        const checksum_t checksum = this->crc.checksum();
        memcpy(buffer, &checksum, numChars);
        std::reverse(std::begin(buffer), std::end(buffer));
        return sender.write(buffer, numChars) + sender.endPacket();
    }

    /// @copydoc    SLIPSender::write()
    size_t write(const uint8_t *data, size_t len);

  private:
    SLIPSender<Sender> sender;
    CRC crc;
};

/// @}

#include "SLIPSender.ipp"
