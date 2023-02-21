#include "assembler/util.h"

namespace secd::assembler::util {

template<>
std::string mangle<value::attribute::Int>(std::string& name) {
    return name + "_const";
}

template<>
std::string mangle<value::attribute::Id>(std::string& name) {
    return name + "_id";
}

template<>
std::string mangle<value::attribute::Code>(std::string& name) {
    return name + "_block";
}

template<>
std::string mangle<value::attribute::Operands>(std::string& name) {
    return name + "_ops";
}

}
