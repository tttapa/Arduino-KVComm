#ifdef ARDUINO

#include <KVComm/include/KV_Builder.hpp>
#include <KVComm/include/KV_Types.hpp>

#else

#include <KV_Builder.hpp>
#include <KV_Types.hpp>

#endif

template <class T>
void KV_Builder::overwrite(uint8_t *buffer, const T *data, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        KV_Type<T>::writeToBuffer(*data++, buffer);
        buffer += KV_Type<T>::getLength();
    }
}

template <class T>
bool KV_Builder::append(const char *key, const T *data, size_t count) {
    uint8_t *dataDestination = writeHeader(key, KV_Type<T>::getTypeID(),
                                           KV_Type<T>::getLength() * count);
    if (dataDestination == nullptr)
        return false;
    overwrite(dataDestination, data, count);
    return true;
}

template <class T>
bool KV_Builder::overwrite(KV_Iterator::iterator existing, const T *data,
                           size_t count) {
    if (existing->getTypeID() != KV_Type<T>::getTypeID() ||
        existing->getDataLength() != KV_Type<T>::getLength() * count)
        return false;
    auto offset          = existing->getData() - buffer;
    uint8_t *destination = buffer + offset;
    overwrite(destination, data, count);
    return true;
}