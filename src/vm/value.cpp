#include "vm/value.h"

std::string secd::value::attribute_name(const attribute& attribute) {
    const char *names[] = {
        "nil",
        "integer",
        "string",
        "boolean",
        "closure",
        "operands list"
    };
    return names[attribute.index()];
}
