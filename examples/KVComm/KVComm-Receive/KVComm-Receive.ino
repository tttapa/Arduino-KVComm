/**
 * Receives SLIP packages containing a KV dictionary and print the keys and 
 * values.
 *
 * @boards  AVR, AVR USB, Nano Every, Nano 33, Due, Teensy 3.x, ESP8266, ESP32
 * 
 * @see     @ref KVComm-Send.ino
 *
 * Written by PieterP, 2020-02-08  
 * https://github.com/tttapa/Arduino-KVComm
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

// Function that is called when a new packet is received
void handlePacket(const uint8_t *data, size_t length);

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

// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

// Print a key-value dictionary entry
template <class T>
void printEntry(const KV_Iterator::KV &entry) {
  Serial.print(entry.getID()), Serial.println(':');
  Serial.print("  ");
  uint16_t numElem = entry.getArraySize<T>();
  for (uint16_t i = 0; i < numElem; ++i)
    Serial.print(entry.getAs<T>(i)), Serial.print(i + 1 < numElem ? ", " : "");
  Serial.println(), Serial.println();
}

// Print a key-value dictionary entry: specialization for strings.
template <>
void printEntry<char>(const KV_Iterator::KV &entry) {
  Serial.print(entry.getID()), Serial.println(':');
  Serial.print("  ");
  Serial.println(entry.getString());
  Serial.println();
}

void handlePacket(const uint8_t *data, size_t length) {
  // Iterate over all key-value pairs in the dictionary
  for (auto &entry : KV_Iterator(data, length)) {
    // Determine the type of the value, and print it
    switch (entry.getTypeID()) {
      case KV_Type<int>::getTypeID(): printEntry<int>(entry); break;
      case KV_Type<float>::getTypeID(): printEntry<float>(entry); break;
      case KV_Type<char>::getTypeID(): printEntry<char>(entry); break;
    }
  }
  Serial.println("====================");
  Serial.println();
}