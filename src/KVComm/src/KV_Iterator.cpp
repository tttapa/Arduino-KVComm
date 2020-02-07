#ifdef ARDUINO

#include <KVComm/include/KVComm/KV_Helpers.hpp> // nextWord, roundUpToWordSizeMultiple
#include <KVComm/include/KVComm/KV_Iterator.hpp>

#include <AH/STL/algorithm> // find_if
#include <string.h>         // strlen

#else

#include <KVComm/KV_Helpers.hpp> // nextWord, roundUpToWordSizeMultiple
#include <KVComm/KV_Iterator.hpp>

#include <algorithm> // find_if
#include <cstring>   // strlen

#endif

KV_Iterator::iterator::iterator() : kv(nullptr), remainingBufferLength(0) {}

KV_Iterator::iterator::iterator(const uint8_t *buffer, size_t length)
    : kv(buffer), remainingBufferLength(length) {
    checkLength();
}

KV_Iterator::iterator &KV_Iterator::iterator::operator++() {
    size_t totalLength = 4 + nextWord(kv.getIDLength()) +
                         roundUpToWordSizeMultiple(kv.getDataLength());
    remainingBufferLength -= totalLength;
    kv = kv.getBuffer() + totalLength;
    checkLength();
    return *this;
}

bool KV_Iterator::iterator::
operator!=(const KV_Iterator::iterator &other) const {
    return this->kv.getBuffer() != other.kv.getBuffer();
}

#if 0
bool KV_Iterator::iterator::
operator==(const KV_Iterator::iterator &other) const {
    return this->kv.getBuffer() == other.kv.getBuffer();
}
#endif

void KV_Iterator::iterator::checkLength() {
    if (remainingBufferLength == 0) {
        kv = nullptr;
    } else if (!kv || kv.getIDLength() == 0) {
        remainingBufferLength = 0;
        kv                    = nullptr;
    }
}

#if !defined(ARDUINO) || defined(DOXYGEN)
std::string KV_Iterator::KV::getString() const {
    if (!checkType<char>())
        return nullptr; // LCOV_EXCL_LINE
    return std::string(getData(), getData() + getDataLength() - 1);
    // -1 because getDataLength() includes null terminator
}
#else
String KV_Iterator::KV::getString() const {
    if (!checkType<char>())
        return static_cast<const char *>(nullptr); // LCOV_EXCL_LINE
    struct S : public String {
        using String::copy;
    } s;
    s.copy(reinterpret_cast<const char *>(getData()), getDataLength() - 1);
    return s;
}
#endif

KV_Iterator::iterator KV_Iterator::find(const char *key) const {
    return std::find_if(begin(), end(), [key](KV_Iterator::KV kv) {
        return strcmp(kv.getID(), key) == 0;
    });
}