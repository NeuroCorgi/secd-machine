#include "assembler/value.h"

namespace secd::assembler::value {

instruction::instruction(secd::opcodes::OpCode opcode, secd::value::operands_list op): opcode{opcode}, operands{std::move(op)} {}

attribute& attribute_list::operator[](uint16_t index) {
    return _attributes[index];
}

uint16_t attribute_list::size() const {
    return _attributes.size();
}
    
}
