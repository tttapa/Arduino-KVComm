#ifndef ARDUINO

#include <KVComm/private/LogEntryIterator.hpp>  // LogEntryIterator
#include <KVComm/public/ParsedLogEntry.hpp>     // ParsedLogEntry

#include <cstring>   // strcmp
#include <iostream>  // cout

ParsedLogEntry::map_t ParsedLogEntry::parse(const uint8_t *buffer,
                                            size_t length) {
    map_t parseResult{};
    for (auto &entry : LogEntryIterator(buffer, length)) {
        const char *identifier = entry.getID();
        // std::cout << entry.getBuffer() - buffer << '\t';
        parseResult.emplace(std::make_pair(identifier, entry));
        // std::cout << identifier << '\t' << +entry.getTypeID() << '\t'
        //           << entry.getDataLength() << std::endl;
    }
    return parseResult;
}

bool ParsedLogEntry::strcmp::operator()(const char *a, const char *b) const {
    return std::strcmp(a, b) < 0;
}

#endif  // ARDUINO