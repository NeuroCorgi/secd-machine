#ifndef SECD_MACHINE_H
#define SECD_MACHINE_H

#include "common.h"

#include "vm/value.h"
#include "vm/opcode.h"
#include "vm/registers.h"

namespace secd {

struct state {
    state(value::attribute_list& attributes);

    bool running {true};

    value::attribute_list attributes;

    registers::stack       stack;
    registers::environment env;
    registers::control     control;
    registers::dump        dump;
};

void run(state&);

} // namespace secd


#endif // SECD_MACHINE_H
