#ifndef ARDUINO

#include <KVComm/KV_Iterator.hpp>  // KV_Iterator
#include <KVComm/KV_Parser.hpp>    // KV_Parser

#include <cstring>   // strcmp
#include <iostream>  // cout

KV_Parser::map_t KV_Parser::parse(const uint8_t *buffer, size_t length) {
    map_t parseResult{};
    for (auto &entry : KV_Iterator(buffer, length)) {
        const char *identifier = entry.getID();
        // std::cout << entry.getBuffer() - buffer << '\t';
        parseResult.emplace(std::make_pair(identifier, entry));
        // std::cout << identifier << '\t' << +entry.getTypeID() << '\t'
        //           << entry.getDataLength() << std::endl;
    }
    return parseResult;
}

bool KV_Parser::strcmp::operator()(const char *a, const char *b) const {
    return std::strcmp(a, b) < 0;
}

#endif  // ARDUINO