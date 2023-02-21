#ifndef SECD_REGISTERS_H
#define SECD_REGISTERS_H

#include <cstdint>
#include <tuple>
#include <stack>
#include <unordered_map>

#include "common.h"

#include "vm/opcode.h"
#include "vm/value.h"

namespace secd::registers {

using stack = std::stack<std::shared_ptr<secd::value::attribute>>;

class control {
public:
    control() = default;
    explicit control(const std::string& control);

    bool empty() const;

    secd::opcodes::OpCode read_opcode();

    uint16_t read_operand();

private:
    void read(char * str, std::size_t count);

    const char* _buffer {nullptr};
    const char* _end    {nullptr};
};

class environment {
public:

    using value_type = std::shared_ptr<secd::value::attribute>;

    const value_type& get(const std::string& id) const;
    value_type& get(const std::string& id);

    void set(const std::string& id, value_type& val);

    static environment derive(std::shared_ptr<environment> parent);

private:
    std::unordered_map<std::string, value_type> _env;
    std::shared_ptr<environment> _parent;
};

using dump = std::stack<std::tuple<stack, environment, control>>;

} // namespace secd::registers

#endif // SECD_REGISTERS_H
