#include <Stream.h>

/// @addtogroup SLIP
/// @{

/**
 * @brief   Class that implements SLIP, a simple packet framing protocol.
 * 
 * [RFC 1055](https://tools.ietf.org/html/rfc1055)
 */
class SLIPStream {
  private:
    /* SLIP special character codes */
    const static uint8_t END     = 0300; ///< indicates end of packet
    const static uint8_t ESC     = 0333; ///< indicates byte stuffing
    const static uint8_t ESC_END = 0334; ///< ESC ESC_END means END data byte
    const static uint8_t ESC_ESC = 0335; ///< ESC ESC_ESC means ESC data byte

  public:
    SLIPStream(Stream &stream) : stream(&stream) {}

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
     * @brief   Set the buffer used by the @ref readPacket function.
     * 
     * @param   buffer
     *          A pointer to the beginning of the buffer.
     * @param   size 
     *          The length of the buffer.
     */
    void setReadBuffer(uint8_t *buffer, size_t size) {
        this->readBuffer = buffer;
        this->bufferSize = size;
        this->received   = 0;
    }

    /**
     * @brief   Check if the received packet was truncated
     */
    bool wasTruncated() const { return truncated; }

  private:
    Stream *stream;
    uint8_t *readBuffer = nullptr;
    size_t bufferSize   = 0;
    size_t received     = 0;
    bool escape         = false;
    bool truncated      = false;
};

/// @}