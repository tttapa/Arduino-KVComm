#include <gtest/gtest.h>

#include <SLIPStream/SLIPStream.hpp>

#include <iostream>
#include <vector>

/* SLIP special character codes */
const uint8_t END     = 0300; ///< indicates end of packet
const uint8_t ESC     = 0333; ///< indicates byte stuffing
const uint8_t ESC_END = 0334; ///< ESC ESC_END means END data byte
const uint8_t ESC_ESC = 0335; ///< ESC ESC_ESC means ESC data byte

struct MockStream : Stream {
    int read() override {
        return readIndex < readLength ? readBuffer[readIndex++] : -1;
    }

    int peek() override {
        return readIndex < readLength ? readBuffer[readIndex] : -1;
    }

    int available() override { return readLength - readIndex; }

    size_t write(uint8_t c) override {
        if (writeIndex < writeLength) {
            writeBuffer[writeIndex++] = c;
            return 1;
        } else {
            return 0;
        }
    }

    const uint8_t *readBuffer = nullptr;
    size_t readLength         = 0;
    size_t readIndex          = 0;

    uint8_t *writeBuffer = nullptr;
    size_t writeLength   = 0;
    size_t writeIndex    = 0;
};

TEST(SLIPStream, send) {
    MockStream stream;
    std::vector<uint8_t> writeBuffer(300);
    stream.writeBuffer = writeBuffer.data();
    stream.writeLength = writeBuffer.size();

    SLIPStream slipstream = {
        stream,
        {nullptr, 0},
    };
    std::vector<uint8_t> packet = {
        0000,    0001,    0002, 0003, 0004, 0005, 0006, 0007, //
        0300,    0301,    0302, 0303, 0304, 0305, 0306, 0307, //
        0330,    0331,    0332, 0333, 0334, 0335, 0336, 0337, //
        END,     END,                                         //
        ESC,     ESC,                                         //
        END,                                                  //
        ESC_ESC, ESC_END,                                     //
    };
    slipstream.writePacket(packet.data(), packet.size());

    std::vector<uint8_t> expected = {
        END,                                                              //
        0000,    0001,    0002, 0003,    0004,    0005, 0006, 0007,       //
        ESC,     ESC_END, 0301, 0302,    0303,    0304, 0305, 0306, 0307, //
        0330,    0331,    0332, ESC,     ESC_ESC, 0334, 0335, 0336, 0337, //
        ESC,     ESC_END, ESC,  ESC_END,                                  //
        ESC,     ESC_ESC, ESC,  ESC_ESC,                                  //
        ESC,     ESC_END,                                                 //
        ESC_ESC, ESC_END,                                                 //
        END,
    };
    writeBuffer.resize(stream.writeIndex);
    EXPECT_EQ(writeBuffer, expected);
}

TEST(SLIPStream, read) {
    MockStream stream;
    std::vector<uint8_t> input = {
        END,                                                              // 1
        0000,    0001,    0002, 0003,    0004,    0005, 0006, 0007,       //
        ESC,     ESC_END, 0301, 0302,    0303,    0304, 0305, 0306, 0307, //
        0330,    0331,    0332, ESC,     ESC_ESC, 0334, 0335, 0336, 0337, //
        ESC,     ESC_END, ESC,  ESC_END,                                  //
        ESC,     ESC_ESC, ESC,  ESC_ESC,                                  //
        ESC,     ESC_END,                                                 //
        ESC_ESC, ESC_END,                                                 //
        END,                                                              //
        END,                                                              // 2
        ESC,     ESC_END, ESC,  ESC_END,                                  //
        ESC,     ESC_ESC, ESC,  ESC_ESC,                                  //
        ESC,     ESC_END,                                                 //
        ESC_ESC, ESC_END,                                                 //
        END,
    };
    stream.readBuffer = input.data();
    stream.readLength = input.size();

    std::vector<uint8_t> packetBuffer(300);
    SLIPStream slipstream = {
        stream,
        {packetBuffer.data(), packetBuffer.size()},
    };

    std::vector<uint8_t> expected1 = {
        0000,    0001,    0002, 0003, 0004, 0005, 0006, 0007, //
        0300,    0301,    0302, 0303, 0304, 0305, 0306, 0307, //
        0330,    0331,    0332, 0333, 0334, 0335, 0336, 0337, //
        END,     END,                                         //
        ESC,     ESC,                                         //
        END,                                                  //
        ESC_ESC, ESC_END,                                     //
    };

    std::vector<uint8_t> expected2 = {
        END,     END,     //
        ESC,     ESC,     //
        END,              //
        ESC_ESC, ESC_END, //
    };

    size_t size1 = slipstream.readPacket();
    packetBuffer.resize(size1);
    EXPECT_EQ(packetBuffer, expected1);
    EXPECT_FALSE(slipstream.wasTruncated());

    packetBuffer.resize(packetBuffer.capacity());

    size_t size2 = slipstream.readPacket();
    packetBuffer.resize(size2);
    EXPECT_EQ(packetBuffer, expected2);
    EXPECT_FALSE(slipstream.wasTruncated());
}

TEST(SLIPStream, readTruncate) {
    MockStream stream;
    std::vector<uint8_t> input = {
        END,                                                        //
        0000, 0001,    0002, 0003, 0004,    0005, 0006, 0007,       //
        ESC,  ESC_END, 0301, 0302, 0303,    0304, 0305, 0306, 0307, //
        0330, 0331,    0332, ESC,  ESC_ESC, 0334, 0335, 0336, 0337, //
        END,
    };
    stream.readBuffer = input.data();
    stream.readLength = input.size();

    std::vector<uint8_t> packetBuffer(24);
    SLIPStream slipstream = {
        stream,
        {packetBuffer.data(), packetBuffer.size() - 1},
    };
    uint8_t guard       = 0111;
    packetBuffer.back() = guard;

    std::vector<uint8_t> expected = {
        0000, 0001, 0002, 0003, 0004, 0005, 0006, 0007,       //
        0300, 0301, 0302, 0303, 0304, 0305, 0306, 0307,       //
        0330, 0331, 0332, 0333, 0334, 0335, 0336, /* 0337, */ //
    };

    size_t size = slipstream.readPacket();
    EXPECT_EQ(packetBuffer.back(), guard);
    packetBuffer.resize(size);
    EXPECT_EQ(packetBuffer, expected);
    EXPECT_TRUE(slipstream.wasTruncated());
}

TEST(SLIPStream, readChunks) {
    MockStream stream;
    std::vector<uint8_t> input = {
        0xC0,                                                 // END
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, // data
        0xDB, 0xDC, 0xDB, 0xDD,                               //
        0xC0,                                                 // END
    };

    std::vector<uint8_t> packetBuffer(64);
    SLIPStream slipstream = {
        stream,                                     // stream
        {packetBuffer.data(), packetBuffer.size()}, // parser
    };

    std::vector<uint8_t> expected = {
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, // data
        0xC0, 0xDB,
    };
    stream.readBuffer = input.data();
    stream.readLength = input.size() - 7;
    // read incomplete packet from input
    EXPECT_EQ(slipstream.readPacket(), 0);
    stream.readLength = input.size();
    // read complete packet from input
    size_t size = slipstream.readPacket();
    packetBuffer.resize(size);
    EXPECT_EQ(packetBuffer, expected);
    EXPECT_FALSE(slipstream.wasTruncated());
}

// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

#include <boost/crc.hpp>

TEST(SLIPStreamCRC, send) {
    MockStream stream;
    std::vector<uint8_t> writeBuffer(300);
    stream.writeBuffer = writeBuffer.data();
    stream.writeLength = writeBuffer.size();

    using CRC = boost::crc_optimal<16, 0x1021, 0xFFFF, 0, false, false>;

    SLIPStreamCRC<CRC> slipstream = {
        stream,       // stream
        CRC(),        // sender CRC
        {nullptr, 0}, // parser
        CRC(),        // parser CRC
    };
    std::vector<uint8_t> data = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
                                 0x37, 0x38, 0x39, END,  ESC};
    slipstream.writePacket(data.data(), data.size());

    std::vector<uint8_t> expected = {
        0xC0,                                                 // END
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, // data
        0xDB, 0xDC, 0xDB, 0xDD,                               //
        0x67, 0xC6,                                           // Checksum
        0xC0,                                                 // END
    };
    writeBuffer.resize(stream.writeIndex);
    EXPECT_EQ(writeBuffer, expected);
}

TEST(SLIPStreamCRC, read) {
    MockStream stream;
    std::vector<uint8_t> input = {
        0xC0,                                                 // END
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, // data
        0xDB, 0xDC, 0xDB, 0xDD,                               //
        0x67, 0xC6,                                           // Checksum
        0xC0,                                                 // END
        0xC0,                                                 // END
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, // data
        0xDB, 0xDC, 0xDB, 0xDD,                               //
        0x67, 0xC6,                                           // Checksum
        0xC0,                                                 // END
    };
    stream.readBuffer = input.data();
    stream.readLength = input.size();

    using CRC = boost::crc_optimal<16, 0x1021, 0xFFFF, 0, false, false>;

    std::vector<uint8_t> packetBuffer(300);

    SLIPStreamCRC<CRC> slipstream = {
        stream,                                     // stream
        CRC(),                                      // sender CRC
        {packetBuffer.data(), packetBuffer.size()}, // parser
        CRC(),                                      // parser CRC
    };

    std::vector<uint8_t> expected1 = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
                                      0x37, 0x38, 0x39, END,  ESC};

    std::vector<uint8_t> expected2 = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
                                      0x37, 0x38, 0x39, END,  ESC};

    size_t size1 = slipstream.readPacket();
    packetBuffer.resize(size1);
    EXPECT_EQ(packetBuffer, expected1);
    EXPECT_EQ(slipstream.checksum(), 0);
    EXPECT_FALSE(slipstream.wasTruncated());

    packetBuffer.resize(packetBuffer.capacity());

    size_t size2 = slipstream.readPacket();
    packetBuffer.resize(size2);
    EXPECT_EQ(packetBuffer, expected2);
    EXPECT_EQ(slipstream.checksum(), 0);
    EXPECT_FALSE(slipstream.wasTruncated());
}

TEST(SLIPStreamCRC, readWrongCRC) {
    MockStream stream;
    std::vector<uint8_t> input = {
        0xC0,                                                 // END
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, // data
        0xDB, 0xDC, 0xDB, 0xDD,                               //
        0x67, 0xC7,                                           // Wrong checksum
        0xC0,                                                 // END
        0xC0,                                                 // END
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, // data
        0xDB, 0xDC, 0xDB, 0xDD,                               //
        0x67, 0xC6,                                           // Checksum
        0xC0,                                                 // END
    };
    stream.readBuffer = input.data();
    stream.readLength = input.size();

    using CRC = boost::crc_optimal<16, 0x1021, 0xFFFF, 0, false, false>;

    std::vector<uint8_t> packetBuffer(300);

    SLIPStreamCRC<CRC> slipstream = {
        stream,                                     // stream
        CRC(),                                      // sender CRC
        {packetBuffer.data(), packetBuffer.size()}, // parser
        CRC(),                                      // parser CRC
    };

    std::vector<uint8_t> expected1 = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
                                      0x37, 0x38, 0x39, END,  ESC};

    std::vector<uint8_t> expected2 = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
                                      0x37, 0x38, 0x39, END,  ESC};

    size_t size1 = slipstream.readPacket();
    packetBuffer.resize(size1);
    EXPECT_EQ(packetBuffer, expected1);
    EXPECT_NE(slipstream.checksum(), 0);
    EXPECT_FALSE(slipstream.wasTruncated());

    packetBuffer.resize(packetBuffer.capacity());

    size_t size2 = slipstream.readPacket();
    packetBuffer.resize(size2);
    EXPECT_EQ(packetBuffer, expected2);
    EXPECT_EQ(slipstream.checksum(), 0);
    EXPECT_FALSE(slipstream.wasTruncated());
}

TEST(SLIPStreamCRC, readTruncate1) {
    MockStream stream;
    std::vector<uint8_t> input = {
        0xC0,                                                 // END
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, // data
        0xDB, 0xDC, 0xDB, 0xDD,                               //
        0x67, 0xC6,                                           // Checksum
        0xC0,                                                 // END
    };
    stream.readBuffer = input.data();
    stream.readLength = input.size();

    using CRC = boost::crc_optimal<16, 0x1021, 0xFFFF, 0, false, false>;

    std::vector<uint8_t> packetBuffer(10 + 1);
    //                                 │   └── extra guard byte
    //                                 └────────── data size - 1
    SLIPStreamCRC<CRC> slipstream = {
        stream,                                         // stream
        CRC(),                                          // sender CRC
        {packetBuffer.data(), packetBuffer.size() - 1}, // parser
        CRC(),                                          // parser CRC
    };
    uint8_t guard       = 0111;
    packetBuffer.back() = guard;

    std::vector<uint8_t> expected = {
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, // data
        0xC0, /* 0xDB, */                                     // truncated
    };

    size_t size = slipstream.readPacket();
    EXPECT_EQ(packetBuffer.back(), guard);
    packetBuffer.resize(size);
    EXPECT_EQ(packetBuffer, expected);
    EXPECT_EQ(slipstream.checksum(), 0);
    EXPECT_EQ(slipstream.numTruncated(), 1);
    EXPECT_TRUE(slipstream.wasTruncated());
}

TEST(SLIPStreamCRC, readTruncate2) {
    MockStream stream;
    std::vector<uint8_t> input = {
        0xC0,                                                 // END
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, // data
        0xDB, 0xDC, 0xDB, 0xDD,                               //
        0x67, 0xC6,                                           // Checksum
        0xC0,                                                 // END
    };
    stream.readBuffer = input.data();
    stream.readLength = input.size();

    using CRC = boost::crc_optimal<16, 0x1021, 0xFFFF, 0, false, false>;

    std::vector<uint8_t> packetBuffer(9 + 1);
    //                                │   └── extra guard byte
    //                                └────────── data size - 2
    SLIPStreamCRC<CRC> slipstream = {
        stream,                                         // stream
        CRC(),                                          // sender CRC
        {packetBuffer.data(), packetBuffer.size() - 1}, // parser
        CRC(),                                          // parser CRC
    };
    uint8_t guard       = 0111;
    packetBuffer.back() = guard;

    std::vector<uint8_t> expected = {
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, // data
        /* 0xC0, 0xDB, */                                     // truncated
    };

    size_t size = slipstream.readPacket();
    EXPECT_EQ(packetBuffer.back(), guard);
    packetBuffer.resize(size);
    EXPECT_EQ(packetBuffer, expected);
    EXPECT_EQ(slipstream.checksum(), 0);
    EXPECT_EQ(slipstream.numTruncated(), 2);
    EXPECT_TRUE(slipstream.wasTruncated());
}

TEST(SLIPStreamCRC, readAlmostTruncated1) {
    MockStream stream;
    std::vector<uint8_t> input = {
        0xC0,                                                 // END
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, // data
        0xDB, 0xDC, 0xDB, 0xDD,                               //
        0x67, 0xC6,                                           // Checksum
        0xC0,                                                 // END
    };
    stream.readBuffer = input.data();
    stream.readLength = input.size();

    using CRC = boost::crc_optimal<16, 0x1021, 0xFFFF, 0, false, false>;

    std::vector<uint8_t> packetBuffer(11 + 1);
    //                                 │   └── extra guard byte
    //                                 └────────── data size
    SLIPStreamCRC<CRC> slipstream = {
        stream,                                         // stream
        CRC(),                                          // sender CRC
        {packetBuffer.data(), packetBuffer.size() - 1}, // parser
        CRC(),                                          // parser CRC
    };
    uint8_t guard       = 0111;
    packetBuffer.back() = guard;

    std::vector<uint8_t> expected = {
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, // data
        0xC0, 0xDB,
    };

    size_t size = slipstream.readPacket();
    EXPECT_EQ(packetBuffer.back(), guard);
    packetBuffer.resize(size);
    EXPECT_EQ(packetBuffer, expected);
    EXPECT_EQ(slipstream.checksum(), 0);
    EXPECT_EQ(slipstream.numTruncated(), 0);
    EXPECT_FALSE(slipstream.wasTruncated());
}

TEST(SLIPStreamCRC, readAlmostTruncated2) {
    MockStream stream;
    std::vector<uint8_t> input = {
        0xC0,                                                 // END
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, // data
        0xDB, 0xDC, 0xDB, 0xDD,                               //
        0x67, 0xC6,                                           // Checksum
        0xC0,                                                 // END
    };
    stream.readBuffer = input.data();
    stream.readLength = input.size();

    using CRC = boost::crc_optimal<16, 0x1021, 0xFFFF, 0, false, false>;

    std::vector<uint8_t> packetBuffer(12 + 1);
    //                                 │   └── extra guard byte
    //                                 └────────── data size + 1
    SLIPStreamCRC<CRC> slipstream = {
        stream,                                         // stream
        CRC(),                                          // sender CRC
        {packetBuffer.data(), packetBuffer.size() - 1}, // parser
        CRC(),                                          // parser CRC
    };
    uint8_t guard       = 0111;
    packetBuffer.back() = guard;

    std::vector<uint8_t> expected = {
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, // data
        0xC0, 0xDB,
    };

    size_t size = slipstream.readPacket();
    EXPECT_EQ(packetBuffer.back(), guard);
    packetBuffer.resize(size);
    EXPECT_EQ(packetBuffer, expected);
    EXPECT_EQ(slipstream.checksum(), 0);
    EXPECT_EQ(slipstream.numTruncated(), 0);
    EXPECT_FALSE(slipstream.wasTruncated());
}

TEST(SLIPStreamCRC, readAlmostTruncated3) {
    MockStream stream;
    std::vector<uint8_t> input = {
        0xC0,                                                 // END
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, // data
        0xDB, 0xDC, 0xDB, 0xDD,                               //
        0x67, 0xC6,                                           // Checksum
        0xC0,                                                 // END
    };
    stream.readBuffer = input.data();
    stream.readLength = input.size();

    using CRC = boost::crc_optimal<16, 0x1021, 0xFFFF, 0, false, false>;

    std::vector<uint8_t> packetBuffer(13 + 1);
    //                                 │   └── extra guard byte
    //                                 └────────── data size + 2
    SLIPStreamCRC<CRC> slipstream = {
        stream,                                         // stream
        CRC(),                                          // sender CRC
        {packetBuffer.data(), packetBuffer.size() - 1}, // parser
        CRC(),                                          // parser CRC
    };
    uint8_t guard       = 0111;
    packetBuffer.back() = guard;

    std::vector<uint8_t> expected = {
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, // data
        0xC0, 0xDB,
    };

    size_t size = slipstream.readPacket();
    EXPECT_EQ(packetBuffer.back(), guard);
    packetBuffer.resize(size);
    EXPECT_EQ(packetBuffer, expected);
    EXPECT_EQ(slipstream.checksum(), 0);
    EXPECT_EQ(slipstream.numTruncated(), 0);
    EXPECT_FALSE(slipstream.wasTruncated());
}

TEST(SLIPStreamCRC, readChunks) {
    MockStream stream;
    std::vector<uint8_t> input = {
        0xC0,                                                 // END
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, // data
        0xDB, 0xDC, 0xDB, 0xDD,                               //
        0x67, 0xC6,                                           // Checksum
        0xC0,                                                 // END
    };

    using CRC = boost::crc_optimal<16, 0x1021, 0xFFFF, 0, false, false>;

    std::vector<uint8_t> packetBuffer(64);
    SLIPStreamCRC<CRC> slipstream = {
        stream,                                     // stream
        CRC(),                                      // sender CRC
        {packetBuffer.data(), packetBuffer.size()}, // parser
        CRC(),                                      // parser CRC
    };

    std::vector<uint8_t> expected = {
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, // data
        0xC0, 0xDB,
    };
    stream.readBuffer = input.data();
    stream.readLength = input.size() - 7;
    // read incomplete packet from input
    EXPECT_EQ(slipstream.readPacket(), 0);
    stream.readLength = input.size();
    // read complete packet from input
    size_t size = slipstream.readPacket();
    packetBuffer.resize(size);
    EXPECT_EQ(packetBuffer, expected);
    EXPECT_EQ(slipstream.checksum(), 0);
    EXPECT_FALSE(slipstream.wasTruncated());
}