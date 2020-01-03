#include <KVComm/private/LogEntryIterator.hpp>

#include <KVComm/private/LoggerHelpers.hpp> // nextWord, roundUpToWordSizeMultiple
#include <cassert>                          // assert
#include <cstring>                          // strlen

LogEntryIterator::LogEntryIterator()
    : buffer(nullptr), remainingBufferLength(0) {}

LogEntryIterator::LogEntryIterator(const uint8_t *buffer, size_t length)
    : buffer(buffer), remainingBufferLength(length) {
    if (!parse())
        remainingBufferLength = 0;
}

LogEntryIterator &LogEntryIterator::operator++() {
    size_t totalLength =
        4 + nextWord(idLength) + roundUpToWordSizeMultiple(dataLength);
    buffer += totalLength;
    assert(totalLength <= remainingBufferLength); // TODO
    remainingBufferLength -= totalLength;
    if (!parse())
        remainingBufferLength = 0;
    return *this;
}

bool LogEntryIterator::parse() {
    if (remainingBufferLength == 0)
        return false;
    idLength = buffer[0];
    if (idLength == 0) // No valid identifier / key
        return false;
    type = buffer[1];
    dataLength = (buffer[2] << 0) | (buffer[3] << 8);
    return true;
}

bool LogEntryIterator::operator!=(const LogEntryIterator &other) {
    return this->remainingBufferLength != other.remainingBufferLength;
}

const uint8_t *LogEntryIterator::getData() const {
    return buffer + nextWord(idLength) + 4;
}