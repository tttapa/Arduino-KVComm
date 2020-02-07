#if !defined(ARDUINO) || defined(DOXYGEN)

#include <KVComm/KV_Iterator.hpp> // KV_Iterator
#include <KVComm/KV_Parser.hpp>   // KV_Parser

#include <cstring>  // strcmp
#include <iostream> // cout

KV_Parser::map_t KV_Parser::parse(const uint8_t *buffer, size_t length) {
    map_t parseResult{};
    for (auto &entry : KV_Iterator(buffer, length)) {
        const char *identifier = entry.getID();
        parseResult.emplace(std::make_pair(identifier, entry));
    }
    return parseResult;
}

bool KV_Parser::strcmp::operator()(const char *a, const char *b) const {
    return std::strcmp(a, b) < 0;
}

#endif // ARDUINO