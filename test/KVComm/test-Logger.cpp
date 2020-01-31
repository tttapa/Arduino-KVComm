#include <gtest/gtest.h>

#include <KVComm/private/LoggerHelpers.hpp>
#include <KVComm/public/Logger.hpp>
#include <KVComm/public/ParsedLogEntry.hpp>

#include <iostream>

TEST(Logger, nextWord) {
    EXPECT_EQ(nextWord(0), 4);
    EXPECT_EQ(nextWord(1), 4);
    EXPECT_EQ(nextWord(2), 4);
    EXPECT_EQ(nextWord(3), 4);
    EXPECT_EQ(nextWord(4), 8);
}

TEST(Logger, roundUpToWordSizeMultiple) {
    EXPECT_EQ(roundUpToWordSizeMultiple(0), 0);
    EXPECT_EQ(roundUpToWordSizeMultiple(1), 4);
    EXPECT_EQ(roundUpToWordSizeMultiple(2), 4);
    EXPECT_EQ(roundUpToWordSizeMultiple(3), 4);
    EXPECT_EQ(roundUpToWordSizeMultiple(4), 4);
    EXPECT_EQ(roundUpToWordSizeMultiple(5), 8);
}

TEST(Logger, logValue) {
    StaticLogger<2048> logger;
    logger.log("value1", (uint32_t) 0xDEADBEEF);
    logger.log("value2", (uint8_t) 0x3C);
    logger.log("value3", (float) 3.14);
    logger.log("key", "value");
    logger.log("🔑", "λ");
    logger.log("bool", true);

    std::vector<uint8_t> expected = {
        0x06, 0x06, 0x04, 0x00,                          // type 6, size 4
        'v',  'a',  'l',  'u',  'e',  '1',  0x00, 0x00,  //
        0xEF, 0xBE, 0xAD, 0xDE,                          // 0xDEADBEEF
        0x06, 0x02, 0x01, 0x00,                          // type 2, size 1
        'v',  'a',  'l',  'u',  'e',  '2',  0x00, 0x00,  //
        0x3C, 0x00, 0x00, 0x00,                          // 0x3C
        0x06, 0x09, 0x04, 0x00,                          // type 9, size 4
        'v',  'a',  'l',  'u',  'e',  '3',  0x00, 0x00,  //
        0xC3, 0xF5, 0x48, 0x40,                          // 3.14f
        0x03, 0x0C, 0x05, 0x00,                          // type 12, size 5
        'k',  'e',  'y',  0x00,                          //
        'v',  'a',  'l',  'u',  'e',  0x00, 0x00, 0x00,  // value
        0x04, 0x0C, 0x02, 0x00,                          // type 12, size 2
        0xF0, 0x9F, 0x94, 0x91, 0x00, 0x00, 0x00, 0x00,  // 🔑
        0xCE, 0xBB, 0x00, 0x00,                          // λ
        0x04, 0x0B, 0x01, 0x00,                          // type 11, size 1
        'b',  'o',  'o',  'l',  0x00, 0x00, 0x00, 0x00,  //
        0x01, 0x00, 0x00, 0x00,                          // true
    };
    const uint8_t *data         = logger.getBuffer();
    size_t length               = logger.getLength();
    std::vector<uint8_t> result = {data, data + length};

    logger.print(std::cout);
    EXPECT_EQ(result, expected);

    ParsedLogEntry parsed = {data, length};
    EXPECT_EQ(parsed["value1"].getAs<uint32_t>(), 0xDEADBEEF);
    EXPECT_EQ(parsed["value2"].getAs<uint8_t>(), 0x3C);
    EXPECT_EQ(parsed["value3"].getAs<float>(), 3.14f);
    EXPECT_EQ(parsed["key"].getString(), "value");
    EXPECT_EQ(parsed["🔑"].getString(), "λ");
    EXPECT_EQ(parsed["bool"].getAs<bool>(), true);
}

TEST(Logger, logArray) {
    StaticLogger<2048> logger;
    float array1[]               = {1.0, 2.0, 3.0};
    std::array<double, 4> array2 = {{-1.0, -2.0, -3.0, -4.0}};
    logger.log("array1", array1);
    logger.log("array2", array2);
    logger.log<int>("array3", {42, 43, 44, 45});

    std::vector<uint8_t> expected = {
        0x06, 0x09, 0x0C, 0x00,                          //
        'a',  'r',  'r',  'a',  'y',  '1',  0x00, 0x00,  //
        0x00, 0x00, 0x80, 0x3F,                          //
        0x00, 0x00, 0x00, 0x40,                          //
        0x00, 0x00, 0x40, 0x40,                          //
        0x06, 0x0A, 0x20, 0x00,                          //
        'a',  'r',  'r',  'a',  'y',  '2',  0x00, 0x00,  //
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xBF,  //
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0,  //
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0xC0,  //
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0xC0,  //
        0x06, 0x05, 0x10, 0x00,                          //
        'a',  'r',  'r',  'a',  'y',  '3',  0x00, 0x00,  //
        0x2A, 0x00, 0x00, 0x00,                          //
        0x2B, 0x00, 0x00, 0x00,                          //
        0x2C, 0x00, 0x00, 0x00,                          //
        0x2D, 0x00, 0x00, 0x00,                          //
    };
    const uint8_t *data         = logger.getBuffer();
    size_t length               = logger.getLength();
    std::vector<uint8_t> result = {data, data + length};

    logger.print(std::cout);
    EXPECT_EQ(result, expected);

    ParsedLogEntry parsed = {data, length};
    for (size_t i = 0; i < 3; ++i)
        EXPECT_EQ(parsed["array1"].getAs<float>(i), array1[i]) << "i = " << i;

    for (size_t i = 0; i < 4; ++i)
        EXPECT_EQ(parsed["array2"].getAs<double>(i), array2[i]) << "i = " << i;

    for (size_t i = 0; i < 4; ++i)
        EXPECT_EQ(parsed["array3"].getAs<int>(i), 42 + i) << "i = " << i;

    std::array<float, 3> array1_expected;
    std::copy(std::begin(array1), std::end(array1), array1_expected.begin());
    std::vector<float> array1_expected_vec = {array1_expected.begin(),
                                              array1_expected.end()};
    auto array1_result     = parsed["array1"].getArray<float, 3>();
    auto array1_result_vec = parsed["array1"].getVector<float>();
    EXPECT_EQ(array1_result, array1_expected);
    EXPECT_EQ(array1_result_vec, array1_expected_vec);

    std::array<double, 4> array2_expected   = array2;
    std::vector<double> array2_expected_vec = {array2_expected.begin(),
                                               array2_expected.end()};
    auto array2_result     = parsed["array2"].getArray<double, 4>();
    auto array2_result_vec = parsed["array2"].getVector<double>();
    EXPECT_EQ(array2_result, array2_expected);
    EXPECT_EQ(array2_result_vec, array2_expected_vec);

    std::array<int, 4> array3_expected   = {{42, 43, 44, 45}};
    std::vector<int> array3_expected_vec = {array3_expected.begin(),
                                            array3_expected.end()};
    auto array3_result                   = parsed["array3"].getArray<int, 4>();
    auto array3_result_vec               = parsed["array3"].getVector<int>();
    EXPECT_EQ(array3_result, array3_expected);
    EXPECT_EQ(array3_result_vec, array3_expected_vec);
}

TEST(Logger, logValueReplace) {
    StaticLogger<2048> logger;
    EXPECT_TRUE(logger.log("value1", (uint32_t) 0xDEADBEEF));
    EXPECT_TRUE(logger.log("value2", (uint8_t) 0x3C));
    EXPECT_TRUE(logger.log("value3", (float) 3.14));
    EXPECT_TRUE(logger.log("value2", (uint8_t) 0x40));  // same id, same type
    EXPECT_FALSE(logger.log("value2", (char) 0x41));    // different type
    uint8_t array[] = {0x42, 0x43};
    EXPECT_FALSE(logger.log("value2", array));  // different length

    std::vector<uint8_t> expected = {
        0x06, 0x06, 0x04, 0x00,                        //
        'v',  'a',  'l',  'u',  'e', '1', 0x00, 0x00,  //
        0xEF, 0xBE, 0xAD, 0xDE,                        //
        0x06, 0x02, 0x01, 0x00,                        //
        'v',  'a',  'l',  'u',  'e', '2', 0x00, 0x00,  //
        0x40, 0x00, 0x00, 0x00,                        //
        0x06, 0x09, 0x04, 0x00,                        //
        'v',  'a',  'l',  'u',  'e', '3', 0x00, 0x00,  //
        0xC3, 0xF5, 0x48, 0x40,                        //
    };
    const uint8_t *data         = logger.getBuffer();
    size_t length               = logger.getLength();
    std::vector<uint8_t> result = {data, data + length};

    logger.print(std::cout);
    EXPECT_EQ(result, expected);

    ParsedLogEntry parsed = {data, length};
    EXPECT_EQ(parsed["value1"].getAs<uint32_t>(), 0xDEADBEEF);
    EXPECT_EQ(parsed["value2"].getAs<uint8_t>(), 0x40);
    EXPECT_EQ(parsed["value3"].getAs<float>(), 3.14f);
}

TEST(ParsedLogEntry, incorrectAccess) {
    StaticLogger<2048> logger;
    EXPECT_TRUE(logger.log("value1", (uint32_t) 0xDEADBEEF));
    EXPECT_TRUE(logger.log<int>("array", {1, 2, 3, 4}));

    const uint8_t *data   = logger.getBuffer();
    size_t length         = logger.getLength();
    ParsedLogEntry parsed = {data, length};

    using out_of_range = AH::ErrorException;
    using logic_error  = AH::ErrorException;
    using length_error = AH::ErrorException;

    // Correct
    EXPECT_NO_THROW(parsed["value1"].getAs<uint32_t>());
    // Incorrect type
    EXPECT_THROW(parsed["value1"].getAs<float>(), logic_error);
    // Incorrect type
    EXPECT_THROW(parsed["value1"].getString(), logic_error);
    // Index out of bounds
    EXPECT_THROW(parsed["value1"].getAs<uint32_t>(1), out_of_range);
    // Non-existing key
    EXPECT_THROW(parsed["value4"].getAs<float>(), std::out_of_range);
    // Correct
    auto method0 = &LogEntryIterator::KV::getArray<int, 4>;
    EXPECT_NO_THROW((parsed["array"].*method0)());
    // Incorrect type
    auto method1 = &LogEntryIterator::KV::getArray<float, 4>;
    EXPECT_THROW((parsed["array"].*method1)(), logic_error);
    // Incorrect size
    auto method2 = &LogEntryIterator::KV::getArray<int, 3>;
    EXPECT_THROW((parsed["array"].*method2)(), length_error);
    // Incorrect size
    auto method3 = &LogEntryIterator::KV::getArray<int, 5>;
    EXPECT_THROW((parsed["array"].*method3)(), length_error);
    // Incorrect type
    EXPECT_THROW(parsed["array"].getVector<float>(), logic_error);
}

TEST(Logger, clearAndReuse) {
    StaticLogger<2048> logger;
    logger.log("value1", (uint32_t) 0xDEADBEEF);
    logger.log("value2", (uint8_t) 0x3C);
    logger.log("value3", (float) 3.14);
    logger.log("key", "value");
    logger.log("🔑", "λ");
    logger.clear();

    std::vector<uint8_t> expected = {};
    const uint8_t *data           = logger.getBuffer();
    size_t length                 = logger.getLength();
    std::vector<uint8_t> result   = {data, data + length};

    EXPECT_EQ(result, expected);

    ParsedLogEntry parsed = {data, length};

    logger.log("key", "value");
    logger.log("🔑", "λ");

    expected = {
        0x03, 0x0C, 0x05, 0x00,                          // type 12, size 5
        'k',  'e',  'y',  0x00,                          //
        'v',  'a',  'l',  'u',  'e',  0x00, 0x00, 0x00,  // value
        0x04, 0x0C, 0x02, 0x00,                          // type 12, size 2
        0xF0, 0x9F, 0x94, 0x91, 0x00, 0x00, 0x00, 0x00,  //
        0xCE, 0xBB, 0x00, 0x00,                          // λ
    };
    data   = logger.getBuffer();
    length = logger.getLength();
    result = {data, data + length};

    logger.print(std::cout);
    EXPECT_EQ(result, expected);

    parsed = {data, length};
    EXPECT_FALSE(parsed.contains("value1"));
    EXPECT_FALSE(parsed.contains("value2"));
    EXPECT_FALSE(parsed.contains("value3"));
    EXPECT_EQ(parsed["key"].getString(), "value");
    EXPECT_EQ(parsed["🔑"].getString(), "λ");
}

TEST(Logger, logValueIntLongShort) {
    int i             = -0x11223344 - 1;
    unsigned u        = 0x44332211;
    long l            = -0x1122334455667788 - 1;
    unsigned long ul  = 0x8877665544332211;
    short s           = -0x1122 - 1;
    unsigned short us = 0x9988;
    int8_t i8         = -0x45;
    uint8_t u8        = 0xAA;
    StaticLogger<2048> logger;
    LOG_VAR(logger, i);
    LOG_VAR(logger, u);
    LOG_VAR(logger, l);
    LOG_VAR(logger, ul);
    LOG_VAR(logger, s);
    LOG_VAR(logger, us);
    LOG_VAR(logger, i8);
    LOG_VAR(logger, u8);

    std::vector<uint8_t> expected = {
        0x01, 0x05, 0x04, 0x00,  //
        'i',  0x00, 0x00, 0x00,  //
        0xBB, 0xCC, 0xDD, 0xEE,  //
        0x01, 0x06, 0x04, 0x00,  //
        'u',  0x00, 0x00, 0x00,  //
        0x11, 0x22, 0x33, 0x44,  //
        0x01, 0x07, 0x08, 0x00,  //
        'l',  0x00, 0x00, 0x00,  //
        0x77, 0x88, 0x99, 0xAA,  //
        0xBB, 0xCC, 0xDD, 0xEE,  //
        0x02, 0x08, 0x08, 0x00,  //
        'u',  'l',  0x00, 0x00,  //
        0x11, 0x22, 0x33, 0x44,  //
        0x55, 0x66, 0x77, 0x88,  //
        0x01, 0x03, 0x02, 0x00,  //
        's',  0x00, 0x00, 0x00,  //
        0xDD, 0xEE, 0x00, 0x00,  //
        0x02, 0x04, 0x02, 0x00,  //
        'u',  's',  0x00, 0x00,  //
        0x88, 0x99, 0x00, 0x00,  //
        0x02, 0x01, 0x01, 0x00,  //
        'i',  '8',  0x00, 0x00,  //
        0xBB, 0x00, 0x00, 0x00,  //
        0x02, 0x02, 0x01, 0x00,  //
        'u',  '8',  0x00, 0x00,  //
        0xAA, 0x00, 0x00, 0x00,  //
    };
    const uint8_t *data         = logger.getBuffer();
    size_t length               = logger.getLength();
    std::vector<uint8_t> result = {data, data + length};

    logger.print(std::cout);
    EXPECT_EQ(result, expected);

    ParsedLogEntry parsed = {data, length};
    EXPECT_EQ(parsed["i"].getAs<decltype(i)>(), i);
    EXPECT_EQ(parsed["u"].getAs<decltype(u)>(), u);
    EXPECT_EQ(parsed["l"].getAs<decltype(l)>(), l);
    EXPECT_EQ(parsed["ul"].getAs<decltype(ul)>(), ul);
    EXPECT_EQ(parsed["s"].getAs<decltype(s)>(), s);
    EXPECT_EQ(parsed["us"].getAs<decltype(us)>(), us);
    EXPECT_EQ(parsed["i8"].getAs<decltype(i8)>(), i8);
    EXPECT_EQ(parsed["u8"].getAs<decltype(u8)>(), u8);
}

TEST(Logger, logValueAllLengths) {
    StaticLogger<2048> logger;
    logger.log("1", std::vector<uint8_t>{0x11});
    logger.log("12", std::vector<uint8_t>{0x11, 0x22});
    logger.log("123", std::vector<uint8_t>{0x11, 0x22, 0x33});
    logger.log("1234", std::vector<uint8_t>{0x11, 0x22, 0x33, 0x44});
    logger.log("12345", std::vector<uint8_t>{0x11, 0x22, 0x33, 0x44, 0x55});
    logger.log("0", std::vector<uint8_t>{});
    logger.log("check", 0xBADBABE);

    std::vector<uint8_t> expected = {
        0x01, 0x02, 0x01, 0x00,  //
        '1',  0x00, 0x00, 0x00,  //
        0x11, 0x00, 0x00, 0x00,  //
        0x02, 0x02, 0x02, 0x00,  //
        '1',  '2',  0x00, 0x00,  //
        0x11, 0x22, 0x00, 0x00,  //
        0x03, 0x02, 0x03, 0x00,  //
        '1',  '2',  '3',  0x00,  //
        0x11, 0x22, 0x33, 0x00,  //
        0x04, 0x02, 0x04, 0x00,  //
        '1',  '2',  '3',  '4',   //
        0x00, 0x00, 0x00, 0x00,  //
        0x11, 0x22, 0x33, 0x44,  //
        0x05, 0x02, 0x05, 0x00,  //
        '1',  '2',  '3',  '4',   //
        '5',  0x00, 0x00, 0x00,  //
        0x11, 0x22, 0x33, 0x44,  //
        0x55, 0x00, 0x00, 0x00,  //
        0x01, 0x02, 0x00, 0x00,  //
        '0',  0x00, 0x00, 0x00,  //
        0x05, 0x05, 0x04, 0x00,  //
        'c',  'h',  'e',  'c',   //
        'k',  0x00, 0x00, 0x00,  //
        0xBE, 0xBA, 0xAD, 0x0B,  //
    };
    const uint8_t *data         = logger.getBuffer();
    size_t length               = logger.getLength();
    std::vector<uint8_t> result = {data, data + length};

    logger.print(std::cout);
    EXPECT_EQ(result, expected);
}