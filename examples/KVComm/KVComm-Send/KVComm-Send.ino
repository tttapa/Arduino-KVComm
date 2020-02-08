/**
 * Send a SLIP package with a dictionary of key-value pairs over the serial port
 * every 5 seconds.
 * 
 * @boards  AVR, AVR USB, Nano Every, Nano 33, Due, Teensy 3.x, ESP8266, ESP32
 * 
 * 1. Upload this sketch to one Arduino.
 * 2. Upload the @ref KVComm-Receive.ino sketch to a second Arduino.
 * 3. Connect the TX pin of this Arduino to the RX pin of the second Arduino.
 * 4. Open the Serial monitor of the second Arduino.
 * 
 * It should print the following:
 * 
 *     ====================
 *     
 *     abc:
 *       1, 2, 3
 *     
 *     password:
 *       qwerty123
 *     
 *     analog 0:
 *       777
 *     
 *     seconds:
 *       12.07
 *     
 *     ====================
 * 
 * For boards where `Serial` is not a hardware UART, use `Serial1` as the stream
 * for the SLIPStream.
 * 
 * Written by PieterP, 2020-02-08  
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

// The dictionary with data to send.
Static_KV_Builder<256> dict;

void setup() {
  Serial.begin(115200);
  // Add some data to the dictionary.
  dict.add("abc", {1, 2, 3});
  dict.add("password", "qwerty123");
}

void loop() {
  // The following will be overwritten on each iteration.
  dict.add("analog 0", analogRead(A0));
  dict.add("seconds", millis() / 1000.0);

  // Send the buffer over the SLIPStream
  slip.writePacket(dict.getBuffer(), dict.getLength());
  delay(2000);
}