#include <KVComm/public/Logger.hpp>
#include <KVComm/public/LoggerTypes.hpp>

template <class T>
void Logger::overwrite(uint8_t *buffer, const T *data, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        LoggableType<T>::writeToBuffer(*data, buffer);
        data++;
        buffer += LoggableType<T>::getLength();
    }
}

template <class T>
bool Logger::append(const char *identifier, const T *data, size_t count) {
    uint8_t *dataDestination =
        writeHeader(identifier, LoggableType<T>::getTypeID(),
                    LoggableType<T>::getLength() * count);
    if (dataDestination == nullptr)
        return false;
    overwrite(dataDestination, data, count);
    return true;
}

template <class T>
bool Logger::overwrite(LogEntryIterator &existing, const T *data,
                       size_t count) {
    if (existing.getTypeID() != LoggableType<T>::getTypeID() ||
        existing.getDataLength() != LoggableType<T>::getLength() * count)
        return false;
    uint8_t *destination = (existing.getData() - buffer) + buffer;
    overwrite(destination, data, count);
    return true;
}