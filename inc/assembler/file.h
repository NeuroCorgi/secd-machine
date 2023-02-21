#ifndef SECD_ASM_FILE_H
#define SECD_ASM_FILE_H

#include <istream>

#include "common.h"

#include "vm/opcode.h"
#include "vm/value.h"

namespace secd::assembler::value {

struct instruction {
    secd::opcodes::OpCode      opcode   {secd::opcodes::NON};
    secd::value::operands_list operands {};

    instruction() = default;
    instruction(secd::opcodes::OpCode, secd::value::operands_list);
    
    friend std::istream& operator>>(std::istream&, instruction&);
};

struct block {
    std::string              label;
    std::vector<instruction> instructions;

    friend std::istream& operator>>(std::istream&, block&);
};

}

#endif // SECD_ASM_FILE_H
