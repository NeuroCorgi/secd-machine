#include "assembler/value.h"

namespace secd::assembler::value {

attribute& attribute_list::operator[](uint16_t index) {
    return _attributes[index];
}

uint16_t attribute_list::size() const {
    return _attributes.size();
}
    
}
