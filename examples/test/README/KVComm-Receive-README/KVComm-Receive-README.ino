/**
 * Don't use this example, it's just for continuous integration tests.
 * 
 * See the KVComm/KVComm-Receive example instead.
 * 
 * @boards  AVR, AVR USB, Nano Every, Nano 33, Due, Teensy 3.x, ESP8266, ESP32
 */

#include <KVComm.h>

#include <SLIPStream/SLIPStream.hpp>
#include <boost/crc.hpp>

// The CRC settings to use, must be the same as the sender.
using CRC = boost::crc_optimal<16, 0x1021, 0xFFFF, 0, false, false>;

// Buffer for saving the incoming packets
uint8_t slipbuffer[256];

// The actual SLIP Stream: it sends and receives packets, adds framing
// bytes, and computes a checksum using the CRC specified above.
SLIPStreamCRC<CRC> slip = {
    Serial,     // stream
    CRC(),      // sender CRC
    slipbuffer, // parser
    CRC(),      // parser CRC
};

void setup() {
    Serial.begin(115200);
    CRC()(0); // initialize the CRC lookup table
}

// Function that is called when a new packet is received
void handlePacket(const uint8_t *data, size_t length) {
    KV_Iterator dict = {data, length};
    auto password = dict.find("password");     // lookup the key "password"
    if (password)                              // if the key exists
        Serial.println(password->getString()); // print the value as a string
}

void loop() {
    // Try to read a packet
    size_t packetSize = slip.readPacket();
    // If a packet was received
    if (packetSize > 0) {
        // Check the integrity of the packet, and make sure it wasn't truncated
        if (slip.checksum() == 0 && !slip.wasTruncated())
            handlePacket(slipbuffer, packetSize);
        else
            Serial.println("<< Invalid packet >>"), Serial.println();
    }
}
