#include "opcode.h"

std::string secd::opcodes::opcode_name(secd::opcodes::OpCode code) {
    switch (code) {
    #define CASE_OPCODE_NAME(name, ...) case name: return #name;
        SECD_OPCODES(CASE_OPCODE_NAME)
    #undef CASE_OPCODE_NAME
    }
}

secd::opcodes::OpCode secd::opcodes::opcode_code(std::string code) {
    #define CASE_OPCODE_NAME(name, hex, _) if (code == #name) return static_cast<secd::opcodes::OpCode>(hex);
        SECD_OPCODES(CASE_OPCODE_NAME)
    #undef CASE_OPCODE_NAME
}