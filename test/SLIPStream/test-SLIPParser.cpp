#include <gtest/gtest.h>

#include <SLIPStream/SLIPParser.hpp>
#include <boost/crc.hpp>

#include <iostream>
#include <vector>

TEST(SLIPParser, parsePacketCRC) {
    std::vector<uint8_t> buffer(64);

    using CRC = boost::crc_optimal<16, 0x1021, 0xFFFF, 0, false, false>;

    SLIPParserCRC<CRC> parser = {{buffer.data(), buffer.size()}, CRC()};

    std::vector<uint8_t> data = {
        0xC0,                                                 // END
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, // data
        0x29, 0xB1,                                           // Checksum
    };
    for (auto c : data)
        EXPECT_EQ(parser.parse(c), 0);
    EXPECT_EQ(parser.parse(SLIP_Constants::END), 9);
    EXPECT_EQ(parser.checksum(), 0);
}