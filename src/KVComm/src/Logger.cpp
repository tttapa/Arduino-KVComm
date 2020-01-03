#include <KVComm/public/Logger.hpp>

#include <AH/STL/limits>                     // std::numeric_limits
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

#ifndef ARDUINO

static inline void printHex(std::ostream &os, uint8_t val) {
    os << nibbleToHex(val >> 4) << nibbleToHex(val);
}

void Logger::print(std::ostream &os) const {
    for (size_t i = 0; i < getLength(); i += 4) {
        os << std::setw(4) << i << "   ";
        for (unsigned int j = 0; j < 4; ++j) {
            printHex(os, buffer[i + j]);
            os << ' ';
        }
        os << "  ";
        for (unsigned int j = 0; j < 4; ++j) {
            os << (isprint(buffer[i + j]) ? (char) buffer[i + j] : '.') << ' ';
        }
        os << std::endl;
    }
}

void Logger::printPython(std::ostream &os) {
    os << "bytes((\n";
    for (size_t i = 0; i < getLength(); i += 4) {
        os << "   ";
        for (unsigned int j = 0; j < 4; ++j) {
            os << " 0x";
            printHex(os, buffer[i + j]);
            os << ",";
        }
        os << "\n";
    }
    os << "))" << std::endl;
}

#endif  // ARDUINO

LogEntryIterator::iterator Logger::find(const char *key) const {
    LogEntryIterator log = {getBuffer(), getLength()};
    return log.find(key);
}