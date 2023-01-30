#ifndef SECD_OPCODE_H
#define SECD_OPCODE_H

#include "common.h"

#include <iostream>

namespace secd::opcodes {

/*
 * SECD machine opcodes
 * name, binary representation, number of arguments fron attribute list
 * 
 * Idea taken fron Webassembly source code
 */

#define SECD_BINARY_OPCODE(V)      \
    V(ADD, 0x05, 0)                \
    V(SUB, 0x06, 0)                \
    V(EQ,  0x09, 0)                \
    V(MUL, 0x0b, 0)                \

#define SECD_OPCODES(V)            \
    SECD_BINARY_OPCODE(V)          \
    V(NON, 0x00, 0)  /* Unused */  \
    V(AP,  0x01, 0)                \
    V(LDC, 0x02, 1)                \
    V(LDE, 0x03, 1)                \
    V(LDF, 0x04, 2)                \
    V(ST,  0x07, 1)                \
    V(NIL, 0x08, 0)                \
    V(SEL, 0x0a, 2)                \
    V(POP, 0x0c, 0)                \
    V(STP, 0xff, 0)                \

enum OpCode {
#define DECLARE_ENUM_NAME(name, opcode, _) name = opcode,
    SECD_OPCODES(DECLARE_ENUM_NAME)
#undef DECLARE_ENUM_NAME
};

std::string opcode_name(OpCode code);
OpCode      opcode_code(std::string code);

constexpr int reference_number(OpCode code) {
    switch (code) {
    #define DECLARE_SWITCH_STATEMENT(name, _, args) case name: return args;
        SECD_OPCODES(DECLARE_SWITCH_STATEMENT)
    #undef DECLARE_SWITCH_STATEMENT
    default:
        throw std::runtime_error(std::to_string(code));
    };
}

constexpr bool is_referencing(OpCode code) {
    return reference_number(code) > 0;
}

} // namespace secd::opcodes

#endif // SECD_OPCODE_H