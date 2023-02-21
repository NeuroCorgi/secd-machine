#ifndef SECD_ASM_UTIL_H
#define SECD_ASM_UTIL_H

#include "common.h"

#include "assembler/attribute.h"

namespace secd::assembler::util {

template<secd::assembler::value::attribute::Tag>
std::string mangle(std::string& name);

template<>
std::string mangle<secd::assembler::value::attribute::Int>(std::string& name);

template<>
std::string mangle<secd::assembler::value::attribute::Id>(std::string& name);

template<>
std::string mangle<secd::assembler::value::attribute::Code>(std::string& name);

template<>
std::string mangle<secd::assembler::value::attribute::Operands>(std::string& name);

}

#endif // SECD_ASM_UTIL_H
