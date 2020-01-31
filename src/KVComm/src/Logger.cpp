#include <KVComm/public/Logger.hpp>

#include <AH/PrintStream/PrintStream.hpp>  // <<

#include <AH/STL/limits>                     // std::numeric_limits
#include <AH/STL/memory>                     // std::make_unique
#include <KVComm/private/LoggerHelpers.hpp>  // nextWord, roundUpToWordSizeMultiple

#ifndef ARDUINO
#include <iomanip>  // setw
#include <ostream>  // os
#endif

uint8_t *Logger::writeHeader(const char *identifier, uint8_t typeID,
                             size_t length) {
    if (length > std::numeric_limits<uint16_t>::max())
        return nullptr;
    size_t idLen = strlen(identifier);
    if (idLen == 0 || idLen > std::numeric_limits<uint8_t>::max())
        return nullptr;
    size_t entryLen = roundUpToWordSizeMultiple(idLen + 1) + 4 +
                      roundUpToWordSizeMultiple(length);
    if (entryLen > maxLen)
        return nullptr;
    bufferwritelocation[0] = idLen;
    bufferwritelocation[1] = typeID;
    bufferwritelocation[2] = length >> 0;
    bufferwritelocation[3] = length >> 8;
    strcpy((char *) bufferwritelocation + 4, identifier);
    size_t dataStartIndex = 4 + nextWord(idLen);
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
    auto str    = AH::make_unique<char[]>(w + 1);
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
void print(const Logger &logger, S &os) {
    for (size_t i = 0; i < logger.getLength(); i += 4) {
        printW(os, i, 4, ' ');
        os << "   ";
        for (uint8_t j = 0; j < 4; ++j) {
            printHex(os, logger.getBuffer()[i + j]);
            os << ' ';
        }
        os << "  ";
        for (uint8_t j = 0; j < 4; ++j) {
            char c = isprint(logger.getBuffer()[i + j])
                         ? (char) logger.getBuffer()[i + j]
                         : '.';
            os << c << ' ';
        }
        os << '\n';
    }
}

template <class S>
void printPython(const Logger &logger, S &os) {
    os << "bytes((\n";
    for (size_t i = 0; i < logger.getLength(); i += 4) {
        os << "   ";
        for (uint8_t j = 0; j < 4; ++j) {
            os << " 0x";
            printHex(os, logger.getBuffer()[i + j]);
            os << ",";
        }
        os << '\n';
    }
    os << "))" << '\n';
}

#ifndef ARDUINO
void Logger::print(std::ostream &os) const { ::print(*this, os); }
void Logger::printPython(std::ostream &os) const { ::printPython(*this, os); }
#endif

void Logger::print(Print &os) const { ::print(*this, os); }
void Logger::printPython(Print &os) const { ::printPython(*this, os); }

LogEntryIterator::iterator Logger::find(const char *key) const {
    LogEntryIterator log = {getBuffer(), getLength()};
    return log.find(key);
}