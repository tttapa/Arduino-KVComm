#include <gtest/gtest.h>

#include <SLIPStream/SLIPSender.hpp>
#include <boost/crc.hpp>

#include <iostream>
#include <vector>

TEST(SLIPSender, writePacketCRC) {
    std::vector<uint8_t> buffer;

    // This is "123456789" in ASCII
    unsigned char const data[] = {0x31, 0x32, 0x33, 0x34, 0x35,
                                  0x36, 0x37, 0x38, 0x39};

    using CRC   = boost::crc_optimal<16, 0x1021, 0xFFFF, 0, false, false>;
    auto sender = [&buffer](uint8_t c) { return buffer.push_back(c), 1; };
    SLIPSenderCRC<decltype(sender), CRC> slipsender(std::move(sender), CRC());

    slipsender.beginPacket();
    slipsender.write(data, sizeof(data));
    slipsender.endPacket();

    buffer.push_back(0xFF); // Guard between packets

    slipsender.beginPacket();
    slipsender.write(data, sizeof(data));
    slipsender.endPacket();

    std::vector<uint8_t> expected = {
        0xC0,                                                 // END
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, // data
        0xB1, 0x29,                                           // Checksum
        0xC0,                                                 // END
        0xFF,                                                 // Guard
        0xC0,                                                 // END
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, // data
        0xB1, 0x29,                                           // Checksum
        0xC0,                                                 // END
    };
    EXPECT_EQ(expected, buffer);
}