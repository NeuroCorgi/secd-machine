#include "machine.h"

#include <iostream>

namespace secd {

state::state(value::attribute_list& attributes) : attributes{attributes} {
    const auto& bytecode = *attributes[attributes.size() - 1];

    if (bytecode.index() != value::attribute_type::Str) {
        throw std::runtime_error("Init : Last element of attributes list must be main code");
    }

    control = registers::control{std::get<std::string>(bytecode)};
}

#define OPCODE_HANDLER(name) void run_##name(state& state, std::vector<uint16_t>& indices)
#define UNUSED(expr) do { (void)(expr); } while (0)

OPCODE_HANDLER(NON) {
    UNUSED(state);
    UNUSED(indices);
}

OPCODE_HANDLER(AP) {
    UNUSED(indices);
    auto& stack = state.stack;

    if (stack.size() < 1) {
        throw std::runtime_error("Ap : Expected a closure on top of the stack : Nothing found");
    }

    auto val = *stack.top(); stack.pop();
    // I prefer to look before I leap
    if (val.index() != value::attribute_type::Closure) {
        throw std::runtime_error("Ap : Expected a closure on top of the stack : " + value::attribute_name(val) + " found");
    }

    auto closure  = std::get<value::closure>(val);
    auto operands = std::get<value::operands_list>(
        *state.attributes[closure.operands_index]
    );
    
    auto env = registers::environment::derive(closure.env);
    for (auto it = operands.crbegin(); it != operands.crend(); ++it) {
        auto ind = *it;
        if (stack.empty()) {
            throw std::runtime_error("Ap : Not enough arguments on the stack");
        }
        env.set(std::get<std::string>(*state.attributes[ind]), stack.top());
        stack.pop();
    }

    auto dump_frame = decltype(state.dump)::value_type(stack, state.env, state.control);

    auto control = std::get<std::string>(*state.attributes[closure.control_index]);

    // Check if closure being called is a built-in function
    if (std::strncmp(control.c_str(), "print", control.capacity() - 1)) {
        
        return;
    }

    state.stack   = registers::stack{};
    state.env     = env;
    state.control = registers::control{control};
    state.dump.push(dump_frame);
}

OPCODE_HANDLER(LDC) {
    uint16_t ind = indices[0];
    if (ind < 0 || ind > state.attributes.size()) {
        throw std::runtime_error("LDC : Operand index out of range");
    }
    state.stack.push(state.attributes[ind]);
}

OPCODE_HANDLER(LDE) {
    uint16_t ind = indices[0];
    if (ind < 0 || ind > state.attributes.size()) {
        throw std::runtime_error("LDE : Operand index out of range");
    }
    auto val = *state.attributes[ind];
    if (val.index() != value::attribute_type::Str) {
        throw std::runtime_error("LDE : Identifier must be a string : " + value::attribute_name(val) + " found");
    }
    auto id = std::get<std::string>(val);
    state.stack.push(state.env.get(id));
}

OPCODE_HANDLER(LDF) {
    auto& attributes = state.attributes;

    value::attribute val;
    value::closure closure;

    if (indices[0] < 0 || indices[0] > attributes.size()) {
        throw std::runtime_error("LDF : Operand index out of range");
    }
    val = *attributes[indices[0]];
    if (val.index() != value::attribute_type::Str) {
        throw std::runtime_error("LDF : Closure control structure must be string : " + value::attribute_name(val) + " found");
    }
    closure.control_index = indices[0];

    if (indices[1] < 0 || indices[1] > attributes.size()) {
        throw std::runtime_error("LDF : Operand index out of range");
    }
    val = *attributes[indices[1]];
    if (val.index() != value::attribute_type::Operands) {
        throw std::runtime_error("LDF : Closure operands list incorrect type : " + value::attribute_name(val) + " found");
    }
    auto op_list = std::get<value::operands_list>(val);
    for (const auto& ind : op_list) {
        if (ind < 0 || ind > attributes.size()) {
            throw std::runtime_error("LDF : Operands list element points out of attributes range");
        }
        if ((*attributes[ind]).index() != value::attribute_type::Str) {
            throw std::runtime_error("LDF : Elements of operands list must be strings : " + value::attribute_name(*attributes[ind]) + " found");
        }
    }
    closure.operands_index = indices[1];
    closure.env = std::make_shared<registers::environment>(state.env);
    state.stack.push(std::make_shared<value::attribute>(closure));
}

OPCODE_HANDLER(ST) {
    uint16_t ind = indices[0];
    if (ind < 0 || ind > state.attributes.size()) {
        throw std::runtime_error("LDE : Operand index out of range");
    }
    auto val = *state.attributes[ind];
    if (val.index() != value::attribute_type::Str) {
        throw std::runtime_error("LDE : Identifier must be a string : " + value::attribute_name(val) + " found");
    }
    auto id = std::get<std::string>(val);
    state.env.set(id, state.stack.top());
    state.stack.pop();
}

OPCODE_HANDLER(SEL) {
    if (state.stack.size() < 1) {
        throw std::runtime_error("SEL : Expected condition value on top of the stack : Nothing found");
    }

    auto condition = *state.stack.top(); state.stack.pop();
    if (condition.index() != value::attribute_type::Bool) {
        throw std::runtime_error("SEL : Condition type must be boolean : " + value::attribute_name(condition) + " found");
    }

    uint16_t branch;
    if (std::get<bool>(condition)) {
        branch = indices[0]; // Then branch
    } else {
        branch = indices[1]; // Else branch
    }

    if (branch < 0 || branch > state.attributes.size()) {
        if (!std::get<bool>(condition)) return;
        throw std::runtime_error("SEL : Branch index is out of range");
    }

    // std::cout << "SEL : " << branch << "\n";

    auto dump_frame = decltype(state.dump)::value_type(state.stack, state.env, state.control);

    auto val = *state.attributes[branch];
    if (val.index() != value::attribute_type::Str) {
        throw std::runtime_error("SEL : Control must be string : " + value::attribute_name(val) + " found");
    }
    state.control = registers::control{
        std::get<std::string>(*state.attributes[branch])
    };
    state.dump.push(dump_frame);
}

OPCODE_HANDLER(NIL) {
    UNUSED(indices);
    state.stack.push(std::make_shared<value::attribute>(nullptr));
}

OPCODE_HANDLER(STP) {
    UNUSED(indices);
    if (state.dump.empty()) {
        state.running = false;
        return;
    }

    auto val = state.stack.top();
    auto [d_stack, d_env, d_control] = state.dump.top(); state.dump.pop();

    state.stack   = d_stack;
    state.env     = d_env;
    state.control = d_control;

    state.stack.push(val);
}

OPCODE_HANDLER(POP) {
    UNUSED(indices);
    if (state.stack.empty()) {
        throw std::runtime_error("POP : Cannot pop from empty stack");
    }
    state.stack.pop();
}

#define BINARY_OPCODE_HANDLER(name, op) \
OPCODE_HANDLER(name) {\
UNUSED(indices);  \
auto& stack = state.stack;   \
if (stack.size() < 2) {   \
throw std::runtime_error(opcodes::opcode_name(opcodes::name) + " : Operation requires 2 arguments"); \
}             \
auto op1 = *stack.top(); stack.pop();            \
auto op2 = *stack.top(); stack.pop();            \
if (op1.index() == op2.index() && op2.index() == value::attribute_type::Int) {  \
value::attribute res =  std::get<int>(op1) op std::get<int>(op2);  \
stack.push(std::make_shared<value::attribute>(res)); \
} else {    \
throw std::runtime_error(opcodes::opcode_name(opcodes::name) + " : Operation requires integers on the stack"); \
}      \
}

BINARY_OPCODE_HANDLER(ADD, +)
BINARY_OPCODE_HANDLER(SUB, -)
BINARY_OPCODE_HANDLER(MUL, *)
BINARY_OPCODE_HANDLER(EQ, ==)

void run(state& state) {
    while (state.running && !state.control.empty()) {
        opcodes::OpCode code = state.control.read_opcode();

        std::vector<uint16_t> operand_indices;
        for (int i = 0; i < opcodes::reference_number(code); ++i) {
            operand_indices.push_back(state.control.read_operand());
        }

        switch (code) {
        #define CALL_OPCODE(name, ...) case opcodes::name: run_##name(state, operand_indices); break;
            SECD_OPCODES(CALL_OPCODE)
        #undef CALL_OPCODE
        }
    }
}

} // namespace secd::machine
