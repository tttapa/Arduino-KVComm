[![Build Status](https://github.com/tttapa/Arduino-KVComm/workflows/CI%20Tests/badge.svg)](https://github.com/tttapa/Arduino-KVComm/actions)
[![Test Coverage](https://img.shields.io/endpoint?url=https://raw.githubusercontent.com/tttapa/Arduino-KVComm/master/docs/Coverage/shield.io.coverage.json)](https://tttapa.github.io/Arduino-KVComm/Coverage/index.html)
[![Build Status](https://travis-ci.org/tttapa/Arduino-KVComm.svg?branch=master)](https://travis-ci.org/tttapa/Arduino-KVComm)
[![GitHub](https://img.shields.io/github/stars/tttapa/Arduino-KVComm?label=GitHub&logo=github)](https://github.com/tttapa/Arduino-KVComm)

# Arduino KVComm

This is a library to write key-value data into a dictionary-like data structure,
in order to communicate between Arduinos, or between an Arduino and a computer.

Packet framing and cyclic redundancy checks are used for error-free transmission
of the data.

The library can be used in the Arduino IDE and as a C++ library for your 
computer.

## Documentation

The automatically generated Doxygen documentation for this library can be found 
here:  
[**Documentation**](https://tttapa.github.io/Arduino-KVComm/Doxygen/index.html)  
Test coverage information can be found here:  
[**Code Coverage**](https://tttapa.github.io/Arduino-KVComm/Coverage/index.html)  
Arduino examples can be found here:  
[**Examples**](https://tttapa.github.io/Arduino-KVComm/Doxygen/examples.html)

It's a good idea to start with the documentation in the 
[**KVComm module**](https://tttapa.github.io/Arduino-KVComm/Doxygen/d4/d09/group__KVComm.html).

## Example usage

The following example adds some key-value data into a dictionary, and sends it
over the Serial port, using the SLIP protocol for packet framing, and a checksum
for data integrity.

```cpp
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
```

As you can see, different data types can be added to the dictionary without any
problems, you can even add arrays and strings.

The following code can be used on the receiver:

```cpp
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
    auto password = dict.find("password");
    if (password)
        Serial.println(password->getString());
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
```

## Supported boards

For each commit, the continuous integration tests compile the examples for the
following boards:

- Arduino UNO
- Arduino Leonardo
- Teensy 3.2
- Arduino Due
- Arduino Nano 33 IoT
- ESP8266
- ESP32

This covers a very large part of the Arduino platform, and similar boards will
also work (e.g. Arduino Nano, Arduino Mega, etc.).

If you have a board that's not supported, please 
[open an issue](https://github.com/tttapa/Arduino-KVComm/issues/new)
and let me know!