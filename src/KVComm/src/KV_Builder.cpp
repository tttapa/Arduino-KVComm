#ifdef ARDUINO

#include <KVComm/include/KV_Helpers.hpp>  // nextWord, roundUpToWordSizeMultiple
#include <KVComm/include/KV_Builder.hpp>

#include <AH/STL/limits>  // std::numeric_limits
#include <AH/STL/memory>  // std::make_unique

#else

#include <KV_Builder.hpp>
#include <KV_Helpers.hpp>  // nextWord, roundUpToWordSizeMultiple

#include <iomanip>  // setw
#include <limits>   // std::numeric_limits
#include <memory>   // std::make_unique
#include <ostream>  // os

#endif

#if defined(ARDUINO) || defined(ARDUINO_TEST)
#include <AH/PrintStream/PrintStream.hpp>  // <<
#endif

uint8_t *KV_Builder::writeHeader(const char *key, uint8_t typeID,
                                 size_t length) {
    if (length > std::numeric_limits<uint16_t>::max())
        return nullptr;
    size_t keyLen = strlen(key);
    if (keyLen == 0 || keyLen > std::numeric_limits<uint8_t>::max())
        return nullptr;
    size_t entryLen = roundUpToWordSizeMultiple(keyLen + 1) + 4 +
                      roundUpToWordSizeMultiple(length);
    if (entryLen > maxLen)
        return nullptr;
    bufferwritelocation[0] = keyLen;
    bufferwritelocation[1] = typeID;
    bufferwritelocation[2] = length >> 0;
    bufferwritelocation[3] = length >> 8;
    strcpy((char *) bufferwritelocation + 4, key);
    size_t dataStartIndex = 4 + nextWord(keyLen);
    uint8_t *dataStart    = bufferwritelocation + dataStartIndex;
    size_t paddedLen      = dataStartIndex + roundUpToWordSizeMultiple(length);
    maxLen -= paddedLen;
    bufferwritelocation += paddedLen;
    if (maxLen > 0)
        bufferwritelocation[0] = 0x00;  // Null terminate
    return dataStart;
}

static inline char nibbleToHex(uint8_t val) {
    val &= 0x0F;
    return val >= 10 ? val + 'A' - 10 : val + '0';
}

template <class S>
static inline void printHex(S &os, uint8_t val) {
    os << nibbleToHex(val >> 4) << nibbleToHex(val);
}

template <class S>
void printW(S &os, unsigned u, uint8_t w, char fill = ' ') {
    auto str    = std::make_unique<char[]>(w + 1);
    str[w]      = '\0';
    char *begin = &str[0];
    char *end   = begin + w - 1;
    do {
        *end-- = u % 10 + '0';
        u /= 10;
    } while (u > 0 && end >= begin);
    if (u > 0) {
        end  = begin + w - 1;
        fill = '*';
    }
    while (end >= begin) {
        *end-- = fill;
    }
    os << begin;
}

template <class S>
void print(const KV_Builder &dict, S &os) {
    for (size_t i = 0; i < dict.getLength(); i += 4) {
        printW(os, i, 4, ' ');
        os << "   ";
        for (uint8_t j = 0; j < 4; ++j) {
            printHex(os, dict.getBuffer()[i + j]);
            os << ' ';
        }
        os << "  ";
        for (uint8_t j = 0; j < 4; ++j) {
            char c = isprint(dict.getBuffer()[i + j])
                         ? (char) dict.getBuffer()[i + j]
                         : '.';
            os << c << ' ';
        }
        os << '\n';
    }
}

template <class S>
void printPython(const KV_Builder &dict, S &os) {
    os << "bytes((\n";
    for (size_t i = 0; i < dict.getLength(); i += 4) {
        os << "   ";
        for (uint8_t j = 0; j < 4; ++j) {
            os << " 0x";
            printHex(os, dict.getBuffer()[i + j]);
            os << ",";
        }
        os << '\n';
    }
    os << "))" << '\n';
}

#ifndef ARDUINO
void KV_Builder::print(std::ostream &os) const { ::print(*this, os); }
void KV_Builder::printPython(std::ostream &os) const {
    ::printPython(*this, os);
}
#endif

#if defined(ARDUINO) || defined(ARDUINO_TEST)
void KV_Builder::print(Print &os) const { ::print(*this, os); }
void KV_Builder::printPython(Print &os) const { ::printPython(*this, os); }
#endif

KV_Iterator::iterator KV_Builder::find(const char *key) const {
    KV_Iterator dict = {getBuffer(), getLength()};
    return dict.find(key);
}