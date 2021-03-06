/**
 * Send a SLIP package over the serial port every 5 seconds.
 * 
 * @boards  AVR, AVR USB, Nano Every, Nano 33, Due, Teensy 3.x, ESP8266, ESP32
 * 
 * 1. Upload this sketch to one Arduino.
 * 2. Upload the @ref SLIP-Receive.ino sketch to a second Arduino.
 * 3. Connect the TX pin of this Arduino to the RX pin of the second Arduino.
 * 4. Open the Serial monitor of the second Arduino.
 * 
 * It should print the following:
 * 
 *     Received packet: 
 *     Hello, world!
 * 
 * For boards where `Serial` is not a hardware UART, use `Serial1` as the stream
 * for the SLIPStream.
 * 
 * Written by PieterP, 2020-02-06  
 * https://github.com/tttapa/Arduino-KVComm
 */

#include <KVComm.h>
#include <SLIPStream/SLIPStream.hpp>

SLIPStream slip = Serial;

void setup() {
  Serial.begin(115200);
}

void loop() {
  uint8_t data[] = "Hello, world!";
  slip.writePacket(data, sizeof(data) - 1);
  delay(5000);
}