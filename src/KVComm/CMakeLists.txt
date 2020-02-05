add_library(kvcomm
    src/KV_Iterator.cpp
    src/KV_Builder.cpp
    src/KV_Parser.cpp
)
target_include_directories(kvcomm 
    PUBLIC
        public
    PRIVATE
        src
        private
)
target_link_libraries(kvcomm PUBLIC ArduinoMock PRIVATE Arduino_Helpers)