#ifndef ARDUINO

#include <KVComm/private/LogEntryIterator.hpp> // LogEntryIterator
#include <KVComm/public/ParsedLogEntry.hpp>    // ParsedLogEntry

#include <cstring>  // strcmp
#include <iostream> // cout

std::string ParsedLogEntry::LogElement::getString() const {
    if (!this->hasType<char>())
        throw std::logic_error("Invalid type: should be char");
    return std::string(this->data, this->data + this->length);
}

ParsedLogEntry::map_t ParsedLogEntry::parse(const uint8_t *buffer,
                                            size_t length) {
    map_t parseResult{};
    for (auto &entry : LogEntryIterator(buffer, length)) {
        const char *identifier = entry.getID();
        // std::cout << entry.getBuffer() - buffer << '\t';
        parseResult[identifier] = {
            entry.getData(),
            entry.getTypeID(),
            entry.getDataLength(),
        };
        // std::cout << identifier << '\t' << +entry.getTypeID() << '\t'
        //           << entry.getDataLength() << std::endl;
    }
    return parseResult;
}

bool ParsedLogEntry::strcmp::operator()(const char *a, const char *b) const {
    return std::strcmp(a, b) < 0;
}

#endif // ARDUINO