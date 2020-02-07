/**
   Receives SLIP packages with a CRC checksum over Serial.

   @see     @ref SLIP-Send-CRC.ino

   Written by PieterP, 2020-02-06
   https://github.com/tttapa/Arduino-KVComm
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
}

void loop() {
  size_t packetSize = slip.readPacket();
  if (packetSize > 0) {
    Serial.print("Received packet: ");
    Serial.write(slipbuffer, packetSize);
    Serial.println();
    Serial.println(slip.wasTruncated() 
                    ? "Size:            Truncated" 
                    : "Size:            OK");
    Serial.println(slip.checksum() != 0 
                    ? "Checksum:        Mismatch" 
                    : "Checksum:        OK");
    Serial.println();
  }
}
