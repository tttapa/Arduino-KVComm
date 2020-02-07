#include <gtest/gtest.h>

#include <SLIPStream/SLIPParser.hpp>
#include <boost/crc.hpp>

#include <iostream>
#include <vector>

TEST(SLIPParser, parsePacketCRC) {
    using namespace SLIP_Constants;
    std::vector<uint8_t> buffer(64);

    using CRC = boost::crc_optimal<16, 0x1021, 0xFFFF, 0, false, false>;

    SLIPParserCRC<CRC> parser = {{buffer.data(), buffer.size()}, CRC()};

    std::vector<uint8_t> packet = {
        0xC0,                                                 // END
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, // data
        0xDB, 0xDC, 0xDB, 0xDD,                               //
        0x67, 0xC6,                                           // Checksum
        // No END
    };
    std::vector<uint8_t> data = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
                                 0x37, 0x38, 0x39, END,  ESC};
    for (auto c : packet)
        EXPECT_EQ(parser.parse(c), 0);
    size_t size = parser.parse(END);
    buffer.resize(size);
    EXPECT_EQ(buffer, data);
    EXPECT_EQ(parser.checksum(), 0);
}