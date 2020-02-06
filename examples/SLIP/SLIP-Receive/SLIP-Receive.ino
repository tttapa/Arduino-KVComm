/**
 * Receives SLIP packages over Serial.
 * 
 * @see     @ref SLIP-Send.ino
 * 
 * Written by PieterP, 2020-02-06  
 * https://github.com/tttapa/Arduino-KVComm
 */

#include <KVComm.h>
#include <SLIPStream/SLIPStream.hpp>

SLIPStream slip = Serial;
uint8_t slipbuffer[256];

void setup() {
    Serial.begin(115200);
    slip.setReadBuffer(slipbuffer, sizeof(slipbuffer));
}

void loop() {
    size_t packetSize = slip.readPacket();
    if (packetSize > 0) {
        Serial.println("Received packet: ");
        Serial.write(slipbuffer, packetSize);
        Serial.println();
        Serial.println(slip.wasTruncated() ? "Packet was truncated\n" : "");
    }
}