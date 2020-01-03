add_library(logger
    src/LogEntryIterator.cpp
    src/Logger.cpp
    src/ParsedLogEntry.cpp
)
target_include_directories(logger 
    PUBLIC
        public
    PRIVATE
        src
        private
)
target_link_libraries(logger PUBLIC ArduinoMock)