#include <KVComm/private/LogEntryIterator.hpp>

#include <AH/STL/algorithm>                  // find_if
#include <KVComm/private/LoggerHelpers.hpp>  // nextWord, roundUpToWordSizeMultiple
#include <assert.h>                          // assert
#include <string.h>                          // strlen

LogEntryIterator::iterator::iterator()
    : kv(nullptr), remainingBufferLength(0) {}

LogEntryIterator::iterator::iterator(const uint8_t *buffer, size_t length)
    : kv(buffer), remainingBufferLength(length) {
    checkLength();
}

LogEntryIterator::iterator &LogEntryIterator::iterator::operator++() {
    size_t totalLength = 4 + nextWord(kv.getIDLength()) +
                         roundUpToWordSizeMultiple(kv.getDataLength());
    remainingBufferLength -= totalLength;
    kv = kv.getBuffer() + totalLength;
    checkLength();
    return *this;
}

bool LogEntryIterator::iterator::
operator!=(const LogEntryIterator::iterator &other) const {
    return this->kv.getBuffer() != other.kv.getBuffer();
}

bool LogEntryIterator::iterator::
operator==(const LogEntryIterator::iterator &other) const {
    return this->kv.getBuffer() == other.kv.getBuffer();
}

void LogEntryIterator::iterator::checkLength() {
    if (!kv || kv.getIDLength() == 0) {
        remainingBufferLength = 0;
        kv                    = nullptr;
    }
}

#ifndef ARDUINO
std::string LogEntryIterator::KV::getString() const {
    if (!checkType<char>())
        return nullptr;
    return std::string(getData(), getData() + getDataLength());
}
#else
String LogEntryIterator::KV::getString() const {
    if (!checkType<char>())
        return static_cast<const char *>(nullptr);
    struct S : public String {
        using String::copy;
    } s;
    s.copy(reinterpret_cast<const char *>(getData()), getDataLength());
    return s;
}
#endif

LogEntryIterator::iterator LogEntryIterator::find(const char *key) const {
    return std::find_if(begin(), end(), [key](LogEntryIterator::KV kv) {
        return strcmp(kv.getID(), key) == 0;
    });
}