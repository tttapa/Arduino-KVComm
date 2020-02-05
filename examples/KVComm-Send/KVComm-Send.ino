/**
 * @boards  AVR, AVR USB, Nano Every, Nano 33, Due, Teensy 3.x, ESP8266, ESP32
 */

#include <KVComm.h>

Static_KV_Builder<256> logger;

void setup() {
  Serial.begin(115200);
  logger.add("test", {1, 2, 3});
  auto test = logger.find("test");
  logger.print(Serial);
  auto testvec = test->getVector<int>();
  for (auto el : testvec)
    Serial.println(el);
}

void loop() {}