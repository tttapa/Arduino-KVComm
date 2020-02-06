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

    SLIPStream slipstream       = stream;
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

    SLIPStream slipstream = stream;
    std::vector<uint8_t> packetBuffer(300);
    slipstream.setReadBuffer(packetBuffer.data(), packetBuffer.size());

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

    packetBuffer.resize(packetBuffer.capacity());

    size_t size2 = slipstream.readPacket();
    packetBuffer.resize(size2);
    EXPECT_EQ(packetBuffer, expected2);
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

    SLIPStream slipstream = stream;
    std::vector<uint8_t> packetBuffer(24);
    uint8_t guard       = 0111;
    packetBuffer.back() = guard;
    slipstream.setReadBuffer(packetBuffer.data(), packetBuffer.size() - 1);

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