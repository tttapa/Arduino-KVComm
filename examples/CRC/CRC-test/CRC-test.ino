/**
 * @boards  AVR, AVR USB, Nano Every, Nano 33, Due, Teensy 3.x, ESP8266, ESP32
 * 
 * Test for boost::crc Arduino port.
 * 
 * Example code from <https://www.boost.org/doc/libs/1_72_0/libs/crc/crc.html>.
 */

#include <KVComm.h>

#include <AH/STL/algorithm>
#include <assert.h>
#include <boost/crc.hpp>

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // This is "123456789" in ASCII
  unsigned char const data[] = {0x31, 0x32, 0x33, 0x34, 0x35,
                                0x36, 0x37, 0x38, 0x39};
  std::size_t const data_len = sizeof(data) / sizeof(data[0]);

  // The expected CRC for the given data
  boost::uint16_t const expected = 0x29B1;

  // Simulate CRC-CCITT
  boost::crc_basic<16> crc_ccitt1(0x1021, 0xFFFF, 0, false, false);
  crc_ccitt1.process_bytes(data, data_len);
  assert(crc_ccitt1.checksum() == expected);

  // Repeat with the optimal version (assuming a 16-bit type exists)
  boost::crc_optimal<16, 0x1021, 0xFFFF, 0, false, false> crc_ccitt2;
  crc_ccitt2 = std::for_each(data, data + data_len, crc_ccitt2);
  assert(crc_ccitt2() == expected);

  Serial.println("All tests passed.");
}

void loop() {}