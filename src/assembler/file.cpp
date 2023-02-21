#include "assembler/file.h"

namespace secd::assembler::value {

instruction::instruction(secd::opcodes::OpCode opcode, secd::value::operands_list op): opcode{opcode}, operands{std::move(op)} {}

}
