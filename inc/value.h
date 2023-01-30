#ifndef VALUE_H
#define VALUE_H

#include <variant>
#include <vector>

#include "common.h"

namespace secd::registers {
class environment;
}

namespace secd::value {

struct closure;

enum attribute_type {
    Nil,
    Int,
    Str,
    Bool,
    Closure,
    Operands,
};

using operands_list = std::vector<uint16_t>;

using attribute = std::variant<nullptr_t, int, std::string, bool, closure, operands_list>;

using attribute_list = std::vector<std::shared_ptr<attribute>>;

struct closure {
    std::shared_ptr<secd::registers::environment> env;
    uint16_t control_index;
    uint16_t operands_index;
};

std::string attribute_name(const attribute& attribute);

} // namespace secd::value

#endif // VALUE_H