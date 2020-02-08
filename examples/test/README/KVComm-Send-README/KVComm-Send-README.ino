/**
 * Don't use this example, it's just for continuous integration tests.
 * 
 * See the KVComm/KVComm-Send example instead.
 * 
 * @boards  AVR, AVR USB, Nano Every, Nano 33, Due, Teensy 3.x, ESP8266, ESP32
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
