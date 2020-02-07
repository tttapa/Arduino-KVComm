/**
 * Send a SLIP package over the serial port every 5 seconds.
 * 
 * 1. Upload this sketch to one Arduino.
 * 2. Upload the @ref SLIP-Receive-CRC.ino sketch to a second Arduino.
 * 3. Connect the TX pin of this Arduino to the RX pin of the second Arduino.
 * 4. Open the Serial monitor of the second Arduino.
 * 
 * It should print the following:
 * 
 *     Received packet: Hello, world!
 *     Size:            OK
 *     Checksum:        OK
 * 
 * Written by PieterP, 2020-02-07  
 * https://github.com/tttapa/Arduino-KVComm
 */

#include <KVComm.h>

#include <SLIPStream/SLIPStream.hpp>
#include <boost/crc.hpp>

// The CRC settings to use, must be the same as the receiver.
using CRC = boost::crc_optimal<16, 0x1021, 0xFFFF, 0, false, false>;

// The actual SLIP Stream: it sends and receives packets, adds framing 
// bytes, and computes a checksum using the CRC specified above.
SLIPStreamCRC<CRC> slip = {
  Serial,  // stream
  CRC(),   // sender CRC
  nullptr, // no parser
  CRC(),   // parser CRC
};

void setup() {
  Serial.begin(115200);
}

void loop() {
  uint8_t data[] = "Hello, world!";
  slip.writePacket(data, sizeof(data) - 1);
  delay(5000);
}
